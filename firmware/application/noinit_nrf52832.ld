/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  /* Put a noinit region at RAM  */
  NOINIT (rwx) : ORIGIN = 0x2000E000, LENGTH = 0x2000
}

SECTIONS
{
  .noinit (NOLOAD):
  {
    /* place all symbols in input sections that start with .noinit */
    KEEP(*(*.noinit*))
  } > NOINIT
}