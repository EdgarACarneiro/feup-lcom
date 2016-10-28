#ifndef __KEYBOARD_H
#define __KEYBOARD_H



/** @defgroup keyboard keyboard
 * @{
 *
 * Functions for using the i8042 KBC/KBD
 */


 /**
  * @brief Subscribes and enables Keyboard interrupts
  *
  * @return Returns bit order in interrupt mask; negative value on failure
  */
 int kbd_subscribe_int(void);

 /**
  * @brief Unsubscribes Keyboard interrupts
  *
  * @return Return 0 upon success and non-zero otherwise
  */
 int kbd_unsubscribe_int(void);

 /**
  * @brief Reads data from the keyboard output buffer
  *
  * @return Return value read upon sucess and -1 otherwise
 */
 //long keyboard_read(void)

 /**
  * @brief Writes data to the keyboard input buffer
  *
  * @param parameter that is written in the input buffer
  *
  * @return Return 0 upon success and non-zero otherwise
  */
 //int keyboard_write(char data)

 /**
  * @brief
  *
  * @param  command of the action we want to execute and the argument that will be used in that action
  *
  * @return Return 0 upon success and non-zero otherwise
  */
 //int keyboard_write_command(char command, char arg)

 /**
  * @brief Toggles the led associated to the received bit
  *
  * @param id that contains information about which bit will be toggled
  *
  * @return Return 0 upon success and non-zero otherwise
  */
 int keyboard_toggle_led (int id,unsigned char *led_status);

 /**
  * @brief Keyboard Interrupt Handler
  *
  * Fetches keyboard output and prints codes in a human friendly way
  *
  * @param pointer to previous iteration's status
  *
  * @return Return 0 upon success and non-zero otherwise
  */
 int keyboard_handler(int * status);


#endif
