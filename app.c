/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdbool.h>
#include "em_common.h"
#include "sl_status.h"
#include "sl_simple_button_instances.h"
#include "sl_simple_led_instances.h"
#include "sl_simple_timer.h"
#include "app_log.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_CATALOG_CLI_PRESENT
#include "sl_cli.h"
#endif // SL_CATALOG_CLI_PRESENT
#include "sl_sensor_rht.h"
#include "sl_health_thermometer.h"
#include "app.h"

#include "MAX31865.h"

// Connection handle.
static uint8_t app_connection = 0;

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

// Button state.
static volatile bool app_btn0_pressed = false;

// Periodic timer handle.
static sl_simple_timer_t app_periodic_timer;

// Periodic timer callback.
static void app_periodic_timer_cb(sl_simple_timer_t *timer, void *data);


#include "spidrv.h"

//SPIDRV_HandleData_t handleData;
//SPIDRV_Handle_t handle = &handleData;
extern SPIDRV_Handle_t sl_spidrv_MAX31865_handle;

void TransferComplete(SPIDRV_Handle_t handle,
                      Ecode_t transferStatus,
                      int itemsTransferred)
{
  if (transferStatus == ECODE_EMDRV_SPIDRV_OK) {
   // Success !
      sl_led_turn_off(&sl_led_led0);
  }
  else
    {
     // sl_led_turn_on(&sl_led_led0);
    }
}



MAX31865 RTDSensor;


void MAX31865_RTD_reconfigure( bool full )
{
  RTDSensor.output_buffer_size = 0;
  /* Write the threshold values. */
  if (full)
  {
    uint16_t threshold ;


    RTDSensor.output_buffer[0] = 0x83;
    RTDSensor.output_buffer_size ++;

    threshold = RTDSensor.configuration_high_threshold ;

    RTDSensor.output_buffer[1] = (( threshold >> 8 ) & 0x00ff );
    RTDSensor.output_buffer_size ++;
    RTDSensor.output_buffer[2] =    threshold        & 0x00ff;
    RTDSensor.output_buffer_size ++;
    threshold = RTDSensor.configuration_low_threshold ;
    RTDSensor.output_buffer[3] = ( ( threshold >> 8 ) & 0x00ff );
    RTDSensor.output_buffer_size ++;
    RTDSensor.output_buffer[4] =   threshold        & 0x00ff;
    RTDSensor.output_buffer_size ++;

  }

  /* Write the configuration to the MAX31865. */
  else
  {
    RTDSensor.output_buffer[0] =  0x80 ;
    RTDSensor.output_buffer_size++;
    RTDSensor.output_buffer[1] = RTDSensor.configuration_control_bits;
    RTDSensor.output_buffer_size++;
  }
  SPIDRV_MTransmit(sl_spidrv_MAX31865_handle, RTDSensor.output_buffer, RTDSensor.output_buffer_size, TransferComplete);
}

/**
 * ReConfigure the MAX31865.  The parameters correspond to Table 2 in the MAX31865
 * datasheet.  The parameters use other control bit-field that is stored internally
 * in the class and change only new values
 *
 * @param [in] v_bias Vbias enabled (@a true) or disabled (@a false).
 * @param [in] conversion_mode Conversion mode auto (@a true) or off (@a false).
 * @param [in] one_shot 1-shot measurement enabled (@a true) or disabled (@a false).
 * @param [in] fault_detection Fault detection cycle control (see Table 3 in the MAX31865
 *             datasheet).
*/
void MAX31865_RTD_configure( bool v_bias, bool conversion_mode, bool one_shot, uint8_t fault_cycle )
{
  /* Use the stored the control bits, and set new ones only */
  RTDSensor.configuration_control_bits &= ~ (0x80 | 0x40 | 0x20 | 0b00001100);

  RTDSensor.configuration_control_bits |= ( v_bias ? 0x80 : 0 );
  RTDSensor.configuration_control_bits |= ( conversion_mode ? 0x40 : 0 );
  RTDSensor.configuration_control_bits |= ( one_shot ? 0x20 : 0 );
  RTDSensor.configuration_control_bits |= fault_cycle & 0b00001100;

  /* Perform light configuration */
  MAX31865_RTD_reconfigure( false );
}





int SPI_APP(void)
{
  uint8_t buffer[10];
//  SPIDRV_Init_t initData = SPIDRV_MASTER_USART2;

  // Initialize an SPI driver instance.
//  SPIDRV_Init(handle, &initData);


  // Transmit data using a callback to catch transfer completion.
  SPIDRV_MTransmit(sl_spidrv_MAX31865_handle, buffer, 10, TransferComplete);
}
/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  app_log_info("health thermometer initialised\n");
  // Init temperature sensor.
  sl_sensor_rht_init();


}

#ifndef SL_CATALOG_KERNEL_PRESENT
/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}
#endif

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  uint8_t system_id[8];

  // Handle stack events
  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:

      sl_led_turn_on(&sl_led_led0);
      MAX31865_RTD_configure( true, true, true, MAX31865_FAULT_DETECTION_NONE);

      // Print boot message.
      app_log_info("Bluetooth stack booted: v%d.%d.%d-b%d\n",
                   evt->data.evt_system_boot.major,
                   evt->data.evt_system_boot.minor,
                   evt->data.evt_system_boot.patch,
                   evt->data.evt_system_boot.build);

      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status(sc);

      // Pad and reverse unique ID to get System ID.
      system_id[0] = address.addr[5];
      system_id[1] = address.addr[4];
      system_id[2] = address.addr[3];
      system_id[3] = 0xFF;
      system_id[4] = 0xFE;
      system_id[5] = address.addr[2];
      system_id[6] = address.addr[1];
      system_id[7] = address.addr[0];

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id);
      app_assert_status(sc);

      app_log_info("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   address_type ? "static random" : "public device",
                   address.addr[5],
                   address.addr[4],
                   address.addr[3],
                   address.addr[2],
                   address.addr[1],
                   address.addr[0]);

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle, // advertising set handle
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start general advertising and enable connections.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      app_log_info("Started advertising\n");
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log_info("Connection opened\n");

#ifdef SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT
      // Set remote connection power reporting - needed for Power Control
      sc = sl_bt_connection_set_remote_power_reporting(
        evt->data.evt_connection_opened.connection,
        sl_bt_connection_power_reporting_enable);
      app_assert_status(sc);
#endif // SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT

      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log_info("Connection closed\n");
      // Restart advertising after client has disconnected.
      sc = sl_bt_advertiser_start(
        advertising_set_handle,
        sl_bt_advertiser_general_discoverable,
        sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      app_log_info("Started advertising\n");
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

/**************************************************************************//**
 * Callback function of connection close event.
 *
 * @param[in] reason Unused parameter required by the health_thermometer component
 * @param[in] connection Unused parameter required by the health_thermometer component
 *****************************************************************************/
void sl_bt_connection_closed_cb(uint16_t reason, uint8_t connection)
{
  (void)reason;
  (void)connection;
  sl_status_t sc;

  // Stop timer.
  sc = sl_simple_timer_stop(&app_periodic_timer);
  app_assert_status(sc);
}

/**************************************************************************//**
 * Health Thermometer - Temperature Measurement
 * Indication changed callback
 *
 * Called when indication of temperature measurement is enabled/disabled by
 * the client.
 *****************************************************************************/
void sl_bt_ht_temperature_measurement_indication_changed_cb(uint8_t connection,
                                                            sl_bt_gatt_client_config_flag_t client_config)
{
  sl_status_t sc;
  app_connection = connection;
  // Indication or notification enabled.
  if (sl_bt_gatt_disable != client_config) {
    // Start timer used for periodic indications.
    sc = sl_simple_timer_start(&app_periodic_timer,
                               SL_BT_HT_MEASUREMENT_INTERVAL_SEC * 1000,
                               app_periodic_timer_cb,
                               NULL,
                               true);
    app_assert_status(sc);
    // Send first indication.
    app_periodic_timer_cb(&app_periodic_timer, NULL);
  }
  // Indications disabled.
  else {
    // Stop timer used for periodic indications.
    (void)sl_simple_timer_stop(&app_periodic_timer);
  }
}

/**************************************************************************//**
 * Simple Button
 * Button state changed callback
 * @param[in] handle Button event handle
 *****************************************************************************/
void sl_button_on_change(const sl_button_t *handle)
{
  // Button pressed.
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (&sl_button_btn0 == handle) {
      sl_led_turn_on(&sl_led_led0);
      app_btn0_pressed = true;
    }
  }
  // Button released.
  else if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_RELEASED) {
    if (&sl_button_btn0 == handle) {
      sl_led_turn_off(&sl_led_led0);
      app_btn0_pressed = false;
    }
  }
}

/**************************************************************************//**
 * Timer callback
 * Called periodically to time periodic temperature measurements and indications.
 *****************************************************************************/
static void app_periodic_timer_cb(sl_simple_timer_t *timer, void *data)
{
  (void)data;
  (void)timer;
  sl_status_t sc;
  int32_t temperature = 0;
  uint32_t humidity = 0;
  float tmp_c = 0.0;
  // float tmp_f = 0.0;

  // Measure temperature; units are % and milli-Celsius.
  sc = sl_sensor_rht_get(&humidity, &temperature);
  if (sc != SL_STATUS_OK) {
    app_log_warning("Invalid RHT reading: %lu %ld\n", humidity, temperature);
  }

  // button 0 pressed: overwrite temperature with -20C.
  if (app_btn0_pressed) {
    temperature = -20 * 1000;
  }

  tmp_c = (float)temperature / 1000;
  app_log_info("Temperature: %5.2f C\n", tmp_c);
  // Send temperature measurement indication to connected client.
  sc = sl_bt_ht_temperature_measurement_indicate(app_connection,
                                                 temperature,
                                                 false);
  // Conversion to Fahrenheit: F = C * 1.8 + 32
  // tmp_f = (float)(temperature*18+320000)/10000;
  // app_log_info("Temperature: %5.2f F\n", tmp_f);
  // Send temperature measurement indication to connected client.
  // sc = sl_bt_ht_temperature_measurement_indicate(app_connection,
  //                                                (temperature*18+320000)/10,
  //                                                true);
  if (sc) {
    app_log_warning("Failed to send temperature measurement indication\n");
  }
}

#ifdef SL_CATALOG_CLI_PRESENT
void hello(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  bd_addr address;
  uint8_t address_type;
  sl_status_t sc = sl_bt_system_get_identity_address(&address, &address_type);
  app_assert_status(sc);
  app_log_info("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
               address_type ? "static random" : "public device",
               address.addr[5],
               address.addr[4],
               address.addr[3],
               address.addr[2],
               address.addr[1],
               address.addr[0]);
}
#endif // SL_CATALOG_CLI_PRESENT
