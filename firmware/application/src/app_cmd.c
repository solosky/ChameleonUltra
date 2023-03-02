#include "fds_util.h"
#include "bsp_time.h"
#include "bsp_delay.h"
#include "usb_main.h"
#include "rfid_main.h"
#include "ble_main.h"
#include "syssleep.h"
#include "tag_emulation.h"
#include "hex_utils.h"
#include "data_cmd.h"
#include "app_cmd.h"
#include "app_status.h"
#include "tag_persistence.h"


#define NRF_LOG_MODULE_NAME app_cmd
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();


data_frame_tx_t* cmd_processor_get_version(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    uint32_t version = 0xDEADBEEF;
    return data_frame_make(cmd, 0xDED1, 4, (uint8_t*)&version);
}

data_frame_tx_t* cmd_processor_change_device_mode(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
#if defined(PROJECT_CHAMELEON_ULTRA)
    if (length == 1) {
        if (data[0] == 1) {
            reader_mode_enter();
        } else {
            tag_mode_enter();
        }
    } else {
        return data_frame_make(cmd, STATUS_PAR_ERR, 0, NULL);
    }
    return data_frame_make(cmd, STATUS_DEVICE_SUCCESS, 0, NULL);
#else
    return data_frame_make(cmd, STATUS_NOT_IMPLEMENTED, 0, NULL);
#endif
}

data_frame_tx_t* cmd_processor_get_device_mode(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    device_mode_t mode = get_device_mode();
    if (mode == DEVICE_MODE_READER) {
        status = 1;
    } else {
        status = 0;
    }
    return data_frame_make(cmd, STATUS_DEVICE_SUCCESS, 1, (uint8_t*)&status);
}


#if defined(PROJECT_CHAMELEON_ULTRA)

data_frame_tx_t* cmd_processor_14a_scan(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    picc_14a_tag_t taginfo;
    status = pcd_14a_reader_scan_auto(&taginfo);
    if (status == HF_TAG_OK) {
        length = sizeof(picc_14a_tag_t);
        data = (uint8_t*)&taginfo;
    } else {
        length = 0;
        data = NULL;
    }
    return data_frame_make(cmd, status, length, data);
}

data_frame_tx_t* cmd_processor_detect_mf1_support(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    status = Check_STDMifareNT_Support();
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_detect_mf1_nt_level(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    status = Check_WeakNested_Support();
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_detect_mf1_darkside(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    status = Check_Darkside_Support();
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_mf1_darkside_acquire(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    DarksideCore dc;
	if (length == 4) {
		status = Darkside_Recover_Key(data[1], data[0], data[2], data[3], &dc);
		if (status == HF_TAG_OK) {
			length = sizeof(DarksideCore);
			data = (uint8_t *)(&dc);
		} else {
			length = 0;
		}
	} else {
		status = STATUS_PAR_ERR;
		length = 0;
	}
    return data_frame_make(cmd, status, length, data);
}

data_frame_tx_t* cmd_processor_detect_nested_dist(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    NestedDist nd;
	if (length == 8) {
		status = Nested_Distacne_Detect(data[1], data[0], &data[2], &nd);
		if (status == HF_TAG_OK) {
			// ̽�����
			length = sizeof(NestedDist);
			data = (uint8_t *)(&nd);
		} else {
			length = 0;
		}
	} else {
		status = STATUS_PAR_ERR;
		length = 0;
	}
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_mf1_nt_distance(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    NestedDist nd;
	if (length == 8) {
		status = Nested_Distacne_Detect(data[1], data[0], &data[2], &nd);
		if (status == HF_TAG_OK) {
			// ̽�����
			length = sizeof(NestedDist);
			data = (uint8_t *)(&nd);
		} else {
			length = 0;
		}
	} else {
		status = STATUS_PAR_ERR;
		length = 0;
	}
    return data_frame_make(cmd, status, length, data);
}

data_frame_tx_t* cmd_processor_mf1_nested_acquire(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    NestedCore ncs[SETS_NR];
	if (length == 10) {
		status = Nested_Recover_Key(bytes_to_num(&data[2], 6), data[1], data[0], data[9], data[8], ncs);
		if (status == HF_TAG_OK) {
			length = sizeof(ncs);
			data = (uint8_t *)(&ncs);
		} else {
			length = 0;
		}
	} else {
		status = STATUS_PAR_ERR;
		length = 0;
	}
    return data_frame_make(cmd, status, length, data);
}

data_frame_tx_t* cmd_processor_mf1_auth_one_key_block(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    if (length == 8) {
		status = auth_key_use_522_hw(data[1], data[0], &data[2]);
        pcd_14a_reader_mf1_unauth();
	} else {
		status = STATUS_PAR_ERR;
	}
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_mf1_read_one_block(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    uint8_t block[16] = { 0x00 };
	if (length == 8) {
		status = auth_key_use_522_hw(data[1], data[0], &data[2]);
		if (status == HF_TAG_OK) {
			// ֱ�ӵ��ñ�׼��ȡAPIȥ��ȡ��Ƭ
			status = pcd_14a_reader_mf1_read(data[1], block);
			if (status == HF_TAG_OK) {
				length = 16;
			} else {
				length = 0;
			}
		} else {
			length = 0;
		}
	} else {
		length = 0;
		status = STATUS_PAR_ERR;
	}
    return data_frame_make(cmd, status, length, block);
}

data_frame_tx_t* cmd_processor_mf1_write_one_block(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    if (length == 24) {
		status = auth_key_use_522_hw(data[1], data[0], &data[2]);
		if (status == HF_TAG_OK) {
			// ֱ�ӵ��ñ�׼д��APIȥд�뿨Ƭ
			status = pcd_14a_reader_mf1_write(data[1], &data[8]);
		} else {
			length = 0;
		}
	} else {
		status = STATUS_PAR_ERR;
	}
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_em410x_scan(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    uint8_t id_buffer[5] = { 0x00 };
	status = PcdScanEM410X(id_buffer);
    return data_frame_make(cmd, status, sizeof(id_buffer), id_buffer);
}

data_frame_tx_t* cmd_processor_write_em410x_2_t57(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    // д��T55XX�ı�ǩ��Ҫ�ṩһ��5��Byte���ȵĿ���
	// ���һ���Ҫ�ṩ����һ���µ���Կ����һ���ɵ���Կ
	if (length >= 13 && (length - 9) % 4 == 0) {
		status = PcdWriteT55XX(
			data, 				// ����UID
			data + 5, 			// ����newkey
			data + 9,			// ����oldkey
			(length - 9) / 4 	// �����ȥnewkey + uid���ʣ���oldkey����Կ����
		);
	} else {
		status = STATUS_PAR_ERR;
	}
    return data_frame_make(cmd, status, 0, NULL);
}

#endif

// ��װһ���Զ��л����۵ĵ��ú���
static void change_slot_auto(uint8_t slot) {
    device_mode_t mode = get_device_mode();
    // ������ģʽ�²���Ҫ����ģ�⿨�ٽ����л�
    tag_emulation_change_slot(slot, mode != DEVICE_MODE_READER);
    // ��������
    light_up_by_slot();
    // Ĭ������RGB
    set_slot_light_color(0);
}

data_frame_tx_t* cmd_processor_set_slot_activated(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    // ��Ҫȷ���������Ŀ��ۺ��벻Ҫ����֧�ֵ�����
    if (length == 1 && data[0] < TAG_MAX_SLOT_NUM) {
        change_slot_auto(data[0]);
        status = STATUS_DEVICE_SUCCESS;
	} else {
        status = STATUS_PAR_ERR;
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_set_slot_tag_type(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    // ��Ҫȷ���������ı�ǩ��������Ч��
    if (length == 2 && data[0] < TAG_MAX_SLOT_NUM && data[1] != TAG_TYPE_UNKNOWN) {
        uint8_t num_slot = data[0];    // ���Ҫ�����Ŀ���
        uint8_t tag_type = data[1];    // ȡ����λ���������ı�ǩ����
        // ����ǰ�Ŀ����л���ָ����ģ�⿨����
        tag_emulation_change_type(num_slot, (tag_specific_type_t)tag_type);
		status = STATUS_DEVICE_SUCCESS;
	} else {
        status = STATUS_PAR_ERR;
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_set_slot_data_default(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    // ��Ҫȷ���������ı�ǩ��������Ч��
    if (length == 2 && data[0] < TAG_MAX_SLOT_NUM && data[1] != TAG_TYPE_UNKNOWN) {
        uint8_t num_slot = data[0];    // ���Ҫ�����Ŀ���
        uint8_t tag_type = data[1];    // ȡ����λ���������ı�ǩ����
        // ���õ�ǰ�Ŀ���Ϊȱʡ���ݣ����ʧ�ܣ�������ǲ�δʵ�ִ�API��ȱʡ
        status = tag_emulation_factory_data(num_slot, (tag_specific_type_t)tag_type) ? STATUS_DEVICE_SUCCESS : STATUS_NOT_IMPLEMENTED;
	} else {
        status = STATUS_PAR_ERR;
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_set_slot_enable(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    // ��Ҫȷ���������ı�ǩ��������Ч��
    if (length == 2 && data[0] < TAG_MAX_SLOT_NUM && (data[1] == 0 || data[1] == 1)) {
        uint8_t slot_now = data[0];  // ���Ҫ�����Ŀ���
        bool enable = data[1];  // ���Ҫ�����Ŀ��۵�״̬
        tag_emulation_slot_set_enable(slot_now, enable);
        if (!enable) {
            // �����˵�ǰ���ۺ�������һ�����õĿ��ۣ��л����Ǹ�������
            uint8_t slot_prev = tag_emulation_slot_find_next(slot_now);
            NRF_LOG_INFO("slot_now = %d, slot_prev = %d", slot_now, slot_prev);
            if (slot_prev == slot_now) {
                // ����һȦ�����ֲ�û���ҵ�ʹ�ܵĿ��ۣ���ô˵��ȫ���Ŀ��۶���������
                // ��ʱ����Ӧ��������۵�
                set_slot_light_color(3);
            } else {
                change_slot_auto(slot_prev);
            }
        }
        status = STATUS_DEVICE_SUCCESS;
	} else {
        status = STATUS_PAR_ERR;
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_slot_data_config_save(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    tag_emulation_save();
    return data_frame_make(cmd, STATUS_DEVICE_SUCCESS, 0, NULL);
}

data_frame_tx_t* cmd_processor_set_em410x_emu_id(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    // ��Ҫȷ����������ID�����ǶԵ�
    if (length == LF_EM410X_TAG_ID_SIZE) {
        // ��ȡEM410x�Ļ�����
        tag_data_buffer_t* buffer = get_buffer_by_tag_type(TAG_TYPE_EM410X);
        // ���ÿ��Ž�ȥ
        memcpy(buffer->buffer, data, LF_EM410X_TAG_ID_SIZE);
        // ����֪ͨ��������
        tag_emulation_load_by_buffer(TAG_TYPE_EM410X, false);
        status = STATUS_DEVICE_SUCCESS;
	} else {
        status = STATUS_PAR_ERR;
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_set_mf1_detection_enable(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    if (length == 1 && (data[0] == 0 || data[0] == 1)) {
        nfc_tag_mf1_detection_log_clear();  // ������Σ�������־�ļ�¼״̬��Ҫ�����־����ʷ��¼
        nfc_tag_mf1_set_detection_enable(data[0]);
        status = STATUS_DEVICE_SUCCESS;
	} else {
        status = STATUS_PAR_ERR;
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_get_mf1_detection_count(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    uint32_t count = nfc_tag_mf1_detection_log_count();
    if (count == 0xFFFFFFFF) {
        count = 0;
    }
    status = STATUS_DEVICE_SUCCESS;
    return data_frame_make(cmd, status, sizeof(uint32_t), (uint8_t *)&count);
}

data_frame_tx_t* cmd_processor_get_mf1_detection_log(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    uint32_t count;
    uint32_t index;
    uint8_t *resp = NULL;
    nfc_tag_mf1_auth_log_t* logs = get_mf1_auth_log(&count);
    if (length == 4) {
        if (count == 0xFFFFFFFF) {
            length = 0;
            status = STATUS_PAR_ERR;
        } else {
            index = bytes_to_num(data, 4);
            // NRF_LOG_INFO("index = %d", index);
            if (index < count) {
                // ֱ��ʹ��ͷ����ַ+index��Ϊ����Դ
                resp = (uint8_t *)(logs + index);
                // ���㵱ǰ���ܴ�����ٸ���־
                length = MIN(count - index, DATA_PACK_MAX_DATA_LENGTH / sizeof(nfc_tag_mf1_auth_log_t));
                // ���㵱ǰ�������־���ֽڳ���
                length = length * sizeof(nfc_tag_mf1_auth_log_t);
                status = STATUS_DEVICE_SUCCESS;
            } else {
                length = 0;
                status = STATUS_PAR_ERR;
            }
        }
    } else {
        length = 0;
        status = STATUS_PAR_ERR;
    }
    return data_frame_make(cmd, status, length, resp);
}

data_frame_tx_t* cmd_processor_set_mf1_emulator_block(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    if (length > 0 && (((length - 1) % NFC_TAG_MF1_DATA_SIZE) == 0)) {
        // ���Ǳ���Ҫȷ����������Ŀ�����û��Խ��
        uint8_t block_index = data[0];
        uint8_t block_count = (length - 1) % NFC_TAG_MF1_DATA_SIZE;
        if (block_index + block_count > NFC_TAG_MF1_BLOCK_MAX) {
            status = STATUS_PAR_ERR;
        } else {
            // Ĭ�ϻ������IC�����ݻ�����
            tag_data_buffer_t* buffer = get_buffer_by_tag_type(TAG_TYPE_MIFARE_4096);
            nfc_tag_mf1_information_t *info = (nfc_tag_mf1_information_t *)buffer->buffer;
            // û��Խ�磬���ǿ��Խ���block����
            for (int i = 1, j = block_index; i < length - 1; i += NFC_TAG_MF1_DATA_SIZE, j++) {
                uint8_t *p_block = &data[i];
                memcpy(info->memory[j], p_block, NFC_TAG_MF1_DATA_SIZE);
            }
            // ������ɣ����ǿ��Ը�֪��λ��
            status = STATUS_DEVICE_SUCCESS;
        }
    } else {
        status = STATUS_PAR_ERR;
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_set_mf1_anti_collision_res(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    if (length > 13) {
        // sak(1) + atqa(2) + uid(10) ���û������������Ⱦ��Բ������13���ֽ�
        status = STATUS_PAR_ERR;
    } else {
        uint8_t uid_length = length - 3;
        if (is_valid_uid_size(uid_length)) {
            nfc_tag_14a_coll_res_referen_t* info = get_miafre_coll_res();
            // copy sak
            info->sak[0] = data[0];
            // copy atqa
            memcpy(info->atqa, &data[1], 2);
            // copy uid
            memcpy(info->uid, &data[3], uid_length);
            // copy size
            *(info->size) = (nfc_tag_14a_uid_size)uid_length;
            status = STATUS_DEVICE_SUCCESS;
        } else {
            // UID���Ȳ���
            status = STATUS_PAR_ERR;
        }
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_set_slot_tag_nick_name(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    // one chinese have 2byte(gbk).
    if (length > 34 || length < 3) {
        status = STATUS_PAR_ERR;
    } else {
        uint8_t slot = data[0];
        uint8_t sense_type = data[1];
        fds_slot_record_map_t map_info;

        get_fds_map_by_slot_sense_type_for_nick(slot, sense_type, &map_info);
        
        uint8_t buffer[36];
        buffer[0] = length - 2; // ��ȥ��ͷ�������ֽڣ�ʣ�µľ����ǳƵ��ֽ���
        memcpy(buffer + 1, data + 2, buffer[0]);    // ����һ���ǳƵ��������У������ӻ���������� һ���ֽڿ�ͷ�ĳ��� + �ǳ��ֽ���
        
        bool ret = fds_write_sync(map_info.id, map_info.key, sizeof(buffer) / 4, buffer);
        if (ret) {
            status = STATUS_DEVICE_SUCCESS;
        } else {
            status = STATUS_FLASH_WRITE_FAIL;
        }
    }
    return data_frame_make(cmd, status, 0, NULL);
}

data_frame_tx_t* cmd_processor_get_slot_tag_nick_name(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    // one chinese have 2byte(gbk).
    if (length != 2) {
        status = STATUS_PAR_ERR;
    } else {
        uint8_t buffer[36];
        uint8_t slot = data[0];
        uint8_t sense_type = data[1];
        fds_slot_record_map_t map_info;

        get_fds_map_by_slot_sense_type_for_nick(slot, sense_type, &map_info);
        bool ret = fds_read_sync(map_info.id, map_info.key, sizeof(buffer), buffer);
        if (ret) {
            status = STATUS_DEVICE_SUCCESS;
            length = buffer[0];
            data = &buffer[1];
        } else {
            status = STATUS_FLASH_READ_FAIL;
            length = 0;
            data = NULL;
        }
    }
    return data_frame_make(cmd, status, length, data);
}

#if defined(PROJECT_CHAMELEON_ULTRA)


/**
 * before reader run, reset reader and on antenna,
 * we must to wait some time, to init picc(power).
 */
data_frame_tx_t* before_reader_run(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    device_mode_t mode = get_device_mode();
    if (mode == DEVICE_MODE_READER) {
        return NULL;
    } else {
        return data_frame_make(cmd, STATUS_DEVIEC_MODE_ERROR, 0, NULL);
    }
}


/**
 * before reader run, reset reader and on antenna,
 * we must to wait some time, to init picc(power).
 */
data_frame_tx_t* before_hf_reader_run(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    data_frame_tx_t* ret = before_reader_run(cmd, status, length, data);
    if (ret == NULL) {
        pcd_14a_reader_reset();
        pcd_14a_reader_antenna_on();
        bsp_delay_ms(8);
    }
    return ret;
}

/**
 * after reader run, off antenna, to keep battery.
 */
data_frame_tx_t* after_hf_reader_run(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    pcd_14a_reader_antenna_off();
    return NULL;
}

#endif

/**
 * (cmd -> processor) function map, the map struct is:
 *       cmd code                               before process               cmd processor                                after process
 */
static cmd_data_map_t m_data_cmd_map[] = {
    {    DATA_CMD_GET_APP_VERSION,              NULL,                        cmd_processor_get_version,                   NULL                   },
    {    DATA_CMD_CHANGE_DEVICE_MODE,           NULL,                        cmd_processor_change_device_mode,            NULL                   },
    {    DATA_CMD_GET_DEVICE_MODE,              NULL,                        cmd_processor_get_device_mode,               NULL                   },

#if defined(PROJECT_CHAMELEON_ULTRA)

    {    DATA_CMD_SCAN_14A_TAG,                 before_hf_reader_run,        cmd_processor_14a_scan,                      after_hf_reader_run    },
    {    DATA_CMD_MF1_SUPPORT_DETECT,           before_hf_reader_run,        cmd_processor_detect_mf1_support,            after_hf_reader_run    },
    {    DATA_CMD_MF1_NT_LEVEL_DETECT,          before_hf_reader_run,        cmd_processor_detect_mf1_nt_level,           after_hf_reader_run    },
    {    DATA_CMD_MF1_DARKSIDE_DETECT,          before_hf_reader_run,        cmd_processor_detect_mf1_darkside,           after_hf_reader_run    },

    {    DATA_CMD_MF1_DARKSIDE_ACQUIRE,         before_hf_reader_run,        cmd_processor_mf1_darkside_acquire,          after_hf_reader_run    },
    {    DATA_CMD_MF1_NT_DIST_DETECT,           before_hf_reader_run,        cmd_processor_mf1_nt_distance,               after_hf_reader_run    },
    {    DATA_CMD_MF1_NESTED_ACQUIRE,           before_hf_reader_run,        cmd_processor_mf1_nested_acquire,            after_hf_reader_run    },

    {    DATA_CMD_MF1_CHECK_ONE_KEY_BLOCK,      before_hf_reader_run,        cmd_processor_mf1_auth_one_key_block,        after_hf_reader_run    },
    {    DATA_CMD_MF1_READ_ONE_BLOCK,           before_hf_reader_run,        cmd_processor_mf1_read_one_block,            after_hf_reader_run    },
    {    DATA_CMD_MF1_WRITE_ONE_BLOCK,          before_hf_reader_run,        cmd_processor_mf1_write_one_block,           after_hf_reader_run    },

    {    DATA_CMD_SCAN_EM410X_TAG,              before_reader_run,           cmd_processor_em410x_scan,                   NULL                   },
    {    DATA_CMD_WRITE_EM410X_TO_T5577,        before_reader_run,           cmd_processor_write_em410x_2_t57,            NULL                   },

#endif

    {    DATA_CMD_SET_SLOT_ACTIVATED,           NULL,                        cmd_processor_set_slot_activated,            NULL                   },
    {    DATA_CMD_SET_SLOT_TAG_TYPE,            NULL,                        cmd_processor_set_slot_tag_type,             NULL                   },
    {    DATA_CMD_SET_SLOT_DATA_DEFAULT,        NULL,                        cmd_processor_set_slot_data_default,         NULL                   },
    {    DATA_CMD_SET_SLOT_ENABLE,              NULL,                        cmd_processor_set_slot_enable,               NULL                   },
    {    DATA_CMD_SLOT_DATA_CONFIG_SAVE,        NULL,                        cmd_processor_slot_data_config_save,         NULL                   },
    

    {    DATA_CMD_SET_EM410X_EMU_ID,            NULL,                        cmd_processor_set_em410x_emu_id,             NULL                   },

    {    DATA_CMD_SET_MF1_DETECTION_ENABLE,     NULL,                        cmd_processor_set_mf1_detection_enable,      NULL                   },
    {    DATA_CMD_GET_MF1_DETECTION_COUNT,      NULL,                        cmd_processor_get_mf1_detection_count,       NULL                   },
    {    DATA_CMD_GET_MF1_DETECTION_RESULT,     NULL,                        cmd_processor_get_mf1_detection_log,         NULL                   },
    {    DATA_CMD_LOAD_MF1_BLOCK_DATA,          NULL,                        cmd_processor_set_mf1_emulator_block,        NULL                   },
    {    DATA_CMD_SET_MF1_ANTI_COLLISION_RES,   NULL,                        cmd_processor_set_mf1_anti_collision_res,    NULL                   },

    {    DATA_CMD_SET_SLOT_TAG_NICK,            NULL,                        cmd_processor_set_slot_tag_nick_name,        NULL                   },
    {    DATA_CMD_GET_SLOT_TAG_NICK,            NULL,                        cmd_processor_get_slot_tag_nick_name,        NULL                   },
};


/**@brief Function for prcoess data frame(cmd)
 */
void on_data_frame_received(uint16_t cmd, uint16_t status, uint16_t length, uint8_t *data) {
    data_frame_tx_t* response = NULL;
    bool is_cmd_support = false;
    // print info
    NRF_LOG_INFO("Data frame: cmd = %02x, status = %02x, length = %d", cmd, status, length);
    NRF_LOG_HEXDUMP_INFO(data, length);
    for (int i = 0; i < ARRAY_SIZE(m_data_cmd_map); i++) {
        if (m_data_cmd_map[i].cmd == cmd) {
            is_cmd_support = true;
            if (m_data_cmd_map[i].cmd_before != NULL) {
                data_frame_tx_t* before_resp = m_data_cmd_map[i].cmd_before(cmd, status, length, data);
                if (before_resp != NULL) {
                    // some problem found before run cmd.
                    response = before_resp;
                    break;
                }
            }
            if (m_data_cmd_map[i].cmd_processor != NULL) response = m_data_cmd_map[i].cmd_processor(cmd, status, length, data);
            if (m_data_cmd_map[i].cmd_after != NULL) {
                data_frame_tx_t* after_resp = m_data_cmd_map[i].cmd_after(cmd, status, length, data);
                if (after_resp != NULL) {
                    // some problem found after run cmd.
                    response = after_resp;
                    break;
                }
            }
            break;
        }
    }
    if (is_cmd_support) {
        // check and response
        if (response != NULL) {
            //usb_cdc_write(response->buffer, response->length);
        }
    } else {
        // response cmd unsupport.
        response = data_frame_make(cmd, STATUS_INVALID_CMD, 0, NULL);
        //usb_cdc_write(response->buffer, response->length);
        NRF_LOG_INFO("Data frame cmd invalid: %d,", cmd);
    }
}
