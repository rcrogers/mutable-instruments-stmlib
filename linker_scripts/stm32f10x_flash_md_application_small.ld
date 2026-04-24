/*
 * STM32F103R8T6
 */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08001000, LENGTH = 124K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
  .text :
  {
    _start = .;
    KEEP(*(.isr_vector))
    *(.text*)
    *(.rodata*)
    *(.flashtext*)
    KEEP (*(.init))
    KEEP (*(.fini))
    _etext = .;
    _sidata = .;
  } > FLASH

  /* Discard exception unwinding info - not needed with -fno-exceptions */
  /DISCARD/ :
  {
    *(.ARM.exidx*)
    *(.ARM.extab*)
  }

  .data : AT (ADDR(.text) + SIZEOF(.text))
  {
    _sdata = .;
    *(.ramtext*)
    . = ALIGN(4);
    *(.data*)
    _edata = .;
  } > RAM

  .bss :
  {
    _sstack = .;
    /*
     * Stack reservation. Stack grows DOWN from _estack; overflow past
     * _sstack silently corrupts .data (function pointer tables like
     * Oscillator::fn_table_, Ui::modes_, CVOutput::dc_fn_table_), which
     * manifests as later HardFaults or silently-wrong behavior.
     *
     * The 2048 B figure is mirrored in yarns/makefile (STACK_RESERVATION)
     * and checked at compile-time against the summed render-path locals in
     * yarns/stack_budget.h. When bumping this number, update the makefile
     * too; the stack_budget.h static_assert will catch the drift if not.
     *
     * Rationale for 2048 (vs the historical 512): yarns's audio render
     * path uses ~930 B cumulative (see stack_budget.h for the per-function
     * breakdown). The historical 512 B was silently absorbed by a libm/libc
     * `.data.impure_data` buffer that happened to sit between yarns's
     * .data and _sstack whenever pow() was linked. Removing that accidental
     * padding revealed the real overflow.
     */
    . = . + 2048;
    _estack = .;
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM
}
