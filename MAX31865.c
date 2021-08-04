/**************************************************************************
 * MAX31865 Basic Example
 *
 *
 *
 * Example code that reads the temperature from an MAX31865 and outputs
 * it on the serial line.
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/



#include "MAX31865.h"
#include "sl_simple_led_instances.h"
#include "app.h"
#include <math.h>


bool transmission_done = 0;

void TransferComplete(SPIDRV_Handle_t handle,
                      Ecode_t transferStatus,
                      int itemsTransferred)
{
  if (transferStatus == ECODE_EMDRV_SPIDRV_OK) {
   // Success !
      sl_led_turn_off(&sl_led_led0);
      transmission_done = 1;
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

    RTDSensor.output_buffer[RTDSensor.output_buffer_size++] = 0x83;
    threshold = RTDSensor.configuration_high_threshold ;
    RTDSensor.output_buffer[RTDSensor.output_buffer_size++] = (( threshold >> 8 ) & 0x00ff );
    RTDSensor.output_buffer[RTDSensor.output_buffer_size++] =    threshold        & 0x00ff;
    threshold = RTDSensor.configuration_low_threshold ;
    RTDSensor.output_buffer[RTDSensor.output_buffer_size++] = ( ( threshold >> 8 ) & 0x00ff );
    RTDSensor.output_buffer[RTDSensor.output_buffer_size++] =   threshold        & 0x00ff;

  }

  /* Write the configuration to the MAX31865. */
  RTDSensor.output_buffer[RTDSensor.output_buffer_size++] =  0x80 ;
  RTDSensor.output_buffer[RTDSensor.output_buffer_size++] = RTDSensor.configuration_control_bits;
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
void MAX31865_RTD_configure_partial( bool v_bias, bool conversion_mode, bool one_shot, uint8_t fault_cycle )
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

/**
 * Configure the MAX31865.  The parameters correspond to Table 2 in the MAX31865
 * datasheet.  The parameters are combined into a control bit-field that is stored
 * internally in the class for later reconfiguration, as are the fault threshold values.
 *
 * @param [in] v_bias Vbias enabled (@a true) or disabled (@a false).
 * @param [in] conversion_mode Conversion mode auto (@a true) or off (@a false).
 * @param [in] one_shot 1-shot measurement enabled (@a true) or disabled (@a false).
 * @param [in] three_wire 3-wire enabled (@a true) or 2-wire/4-wire (@a false).
 * @param [in] fault_detection Fault detection cycle control (see Table 3 in the MAX31865
 *             datasheet).
 * @param [in] fault_clear Fault status auto-clear (@a true) or manual clear (@a false).
 * @param [in] filter_50hz 50 Hz filter enabled (@a true) or 60 Hz filter enabled
 *             (@a false).
 * @param [in] low_threshold Low fault threshold.
 * @param [in] high_threshold High fault threshold.
*/
void MAX31865_RTD_configure( bool v_bias, bool conversion_mode, bool one_shot,
                              bool three_wire, uint8_t fault_cycle, bool fault_clear,
                              bool filter_50hz, uint16_t low_threshold,
                              uint16_t high_threshold )
{
  uint8_t control_bits = 0;

  /* Assemble the control bit mask. */
  control_bits |= ( v_bias ? 0x80 : 0 );
  control_bits |= ( conversion_mode ? 0x40 : 0 );
  control_bits |= ( one_shot ? 0x20 : 0 );
  control_bits |= ( three_wire ? 0x10 : 0 );
  control_bits |= fault_cycle & 0b00001100;
  control_bits |= ( fault_clear ? 0x02 : 0 );
  control_bits |= ( filter_50hz ? 0x01 : 0 );

  /* Store the control bits and the fault threshold limits for reconfiguration
     purposes. */
  RTDSensor.configuration_control_bits   = control_bits;
  RTDSensor.configuration_low_threshold  = low_threshold;
  RTDSensor.configuration_high_threshold = high_threshold;

  /* Perform an full reconfiguration */
  MAX31865_RTD_reconfigure( true );
}

/**
 * Read all settings and measurements from the MAX31865 and store them
 * internally in the class.
 *
 * @return Fault status byte
 */
uint8_t MAX31865_RTD_read_all()
{
  uint16_t combined_bytes;

  /* The following MAX31865 registers will be read:
       Configuration
       RTD
       High Fault Threshold
       Low Fault Threshold
       Fault Status */

  RTDSensor.output_buffer_size = 0;

  //Filling the output buffer with 0's, in order to read data back.
  while (RTDSensor.output_buffer_size < 8)
    {
      RTDSensor.output_buffer[RTDSensor.output_buffer_size++] = 0;
    }

  transmission_done = 0;
  /* Tell the MAX31865 that we want to read, starting at register 0. */
  //Non Blocking call to transfer data over SPI
  //SPIDRV_MTransfer( sl_spidrv_MAX31865_handle,RTDSensor.output_buffer,RTDSensor.input_buffer,RTDSensor.output_buffer_size,TransferComplete);

  //Blocking call to transfer data over SPI
  SPIDRV_MTransferB( sl_spidrv_MAX31865_handle,RTDSensor.output_buffer,RTDSensor.input_buffer,RTDSensor.output_buffer_size);


  RTDSensor.measured_configuration = RTDSensor.input_buffer[1];

  combined_bytes  = RTDSensor.input_buffer[2] << 8;
  combined_bytes |= RTDSensor.input_buffer[3];
  RTDSensor.measured_resistance = combined_bytes >> 1;

  combined_bytes  = RTDSensor.input_buffer[4] << 8;
  combined_bytes |= RTDSensor.input_buffer[5];
  RTDSensor.measured_high_threshold = combined_bytes ;

  combined_bytes  = RTDSensor.input_buffer[6] << 8;
  combined_bytes |= RTDSensor.input_buffer[7];
  RTDSensor.measured_low_threshold = combined_bytes ;

  RTDSensor.measured_status = RTDSensor.input_buffer[8];



  /* Reset the configuration if the measured resistance is
     zero or a fault occurred. */
  if(   RTDSensor.measured_resistance == 0 || RTDSensor.measured_status != 0  )
    MAX31865_RTD_reconfigure( true );

  return( RTDSensor.measured_status );
}

uint8_t MAX31865_RTD_fault_status()
{
  uint8_t fault_status ;

  /* Initializes Byte Counter for Transmission/Receive process */
  RTDSensor.output_buffer_size = 0;
  /* Tell the MAX31865 that we want to read, starting at register 7. */
  RTDSensor.output_buffer[RTDSensor.output_buffer_size++] = 0x07;
  RTDSensor.output_buffer[RTDSensor.output_buffer_size++] = 0x00;
  //Blocking call to transfer data over SPI
  SPIDRV_MTransferB( sl_spidrv_MAX31865_handle,RTDSensor.output_buffer,RTDSensor.input_buffer,RTDSensor.output_buffer_size);

  fault_status = RTDSensor.input_buffer[1];

  return( fault_status );
}


/**
 * The constructor for the MAX31865_RTD class registers the CS pin and
 * configures it as an output.
 *
 * @param [in] type type of RTD .
 * @param [in] cs_pin Arduino pin selected for the CS signal.
 * @param [in] rtd_rref reference resistor value. if omitted will be setup to default
              400 for PT100 and 4000 for PT1000
 */
void MAX31865_RTD_Sensor_Type( PTDTYPE type)
{
  /* Set the type of PTD. */
  RTDSensor.type = type;

    RTDSensor.configuration_rtd_rref = type == RTD_PT100 ? RTD_RREF_PT100 : RTD_RREF_PT1000;
}

uint16_t raw_resistance( )
{ return(  RTDSensor.measured_resistance ); }

double resistance( )
{
  return( (double)raw_resistance( ) * RTDSensor.configuration_rtd_rref / (double)RTD_ADC_RESOLUTION );
}

/**
 * Apply the Callendar-Van Dusen equation to convert the RTD resistance
 * to temperature:
 *
 *   \f[
 *   t=\frac{-A\pm \sqrt{A^2-4B\left(1-\frac{R_t}{R_0}\right)}}{2B}
 *   \f],
 *
 * where
 *
 * \f$A\f$ and \f$B\f$ are the RTD coefficients, \f$R_t\f$ is the current
 * resistance of the RTD, and \f$R_0\f$ is the resistance of the RTD at 0
 * degrees Celcius.
 *
 * For more information on measuring with an RTD, see:
 * <http://newton.ex.ac.uk/teaching/CDHW/Sensors/an046.pdf>.
 *
 * @param [in] resistance The measured RTD resistance.
 * @return Temperature in degrees Celcius.
 */
double MAX31865_RTD_temperature()
{
  static const double a2   = 2.0 * RTD_B;
  static const double b_sq = RTD_A * RTD_A;

  const double rtd_resistance = ( RTDSensor.configuration_rtd_rref == RTD_PT100 )? RTD_RESISTANCE_PT100 : RTD_RESISTANCE_PT1000;

  double c = 1.0 - resistance( ) / rtd_resistance;
  double D = b_sq - 2.0 * a2 * c;
  double temperature_deg_C = ( -RTD_A + sqrt( D ) ) / a2;

  return( temperature_deg_C );
}
