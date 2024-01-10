/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0001B000, LENGTH = 0x15000

  RTT_CB (rwx) :  ORIGIN = 0x20002B00, LENGTH = 0x100

  RAM (rwx) :  ORIGIN = 0x20002C00, LENGTH = 0x5300

  /** Location of non initialized RAM. Non initialized RAM is used for exchanging bond information
   *  from application to bootloader when using buttonless DFU OTA.
   */
  NOINIT (rwx) :  ORIGIN = 0x20007F80, LENGTH = 0x80

  /** Location of bootloader setting in at the last flash page. */
  BOOTLOADER_SETTINGS (rw) : ORIGIN = 0x0003FC00, LENGTH = 0x0400

  uicr_bootloader_start_address (r) : ORIGIN = 0x10001014, LENGTH = 0x4

}

SECTIONS
{
  . = ALIGN(4);
  .uicr_bootloader_start_address :
  {
    PROVIDE(__start_uicr_bootloader_start_address = .);
    KEEP(*(SORT(.uicr_bootloader_start_address*)))
    PROVIDE(__stop_uicr_bootloader_start_address = .);
  } > uicr_bootloader_start_address
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(.pwr_mgmt_data))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > RAM

  .rttSection :
  {
    SEGGER_RTT.c.*(.bss._SEGGER_RTT)
  } > RTT_CB

  .noinit(NOLOAD) :
  {
    KEEP(*(.noinit))
  } > NOINIT

} INSERT AFTER .data;

INCLUDE "nrf5x_common.ld"
