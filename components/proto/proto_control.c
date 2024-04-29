#include "proto_control.h"

struct VehicleData to_be_serialized = {
    .isEmergency = 1,
    .isHydrogenLeaking = 0,
    .isScRelayClosed = 0,
    .vehicleIsSpeedButtonPressed = 0,
    .vehicleIsHalfSpeedButtonPressed = 1,
    .hydrogenCellOneButtonState = 1,
    .hydrogenCellTwoButtonState = 1,
    .isSuperCapacitorButtonPressed = 1,
    .logicState = 12,
    .fcCurrent = 2.1,
    .fcScCurrent = 2.2,
    .scMotorCurrent = 2.3,
    .fcVoltage = 2.4,
    .scVoltage = 2.5,
    .hydrogenSensorVoltage = 2.6,
    .fuelCellTemperature = 2.7,
    .fanRpm = 5,
    .vehicleSpeed = 2.8,
    .motorPwm = 5,
    .hydrogenPressure = 2.9,
};
ssize_t buffer_len = 0;
uint8_t buf[4096];
void hexDump(const char *desc, const void *addr, const int len, int perLine) {
  // Silently ignore silly per-line values.

  if (perLine < 4 || perLine > 64)
    perLine = 16;

  int i;
  unsigned char buff[perLine + 1];
  const unsigned char *pc = (const unsigned char *)addr;

  // Output description if given.

  if (desc != NULL)
    printf("%s:\n", desc);

  // Length checks.

  if (len == 0) {
    printf("  ZERO LENGTH\n");
    return;
  }
  if (len < 0) {
    printf("  NEGATIVE LENGTH: %d\n", len);
    return;
  }

  // Process every byte in the data.

  for (i = 0; i < len; i++) {
    // Multiple of perLine means new or first line (with line offset).

    if ((i % perLine) == 0) {
      // Only print previous-line ASCII buffer for lines beyond first.

      if (i != 0)
        printf("  %s\n", buff);

      // Output the offset of current line.

      printf("  %04x ", i);
    }

    // Now the hex code for the specific character.

    printf(" %02x", pc[i]);

    // And buffer a printable ASCII character for later.

    if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
      buff[i % perLine] = '.';
    else
      buff[i % perLine] = pc[i];
    buff[(i % perLine) + 1] = '\0';
  }

  // Pad out last line if not exactly perLine characters.

  while ((i % perLine) != 0) {
    printf("   ");
    i++;
  }

  // And print the final ASCII buffer.

  printf("  %s\n", buff);
}

void serialize_vehicle_data(vehicle_data_t vehicle_state_data, uint8_t *serialized_vehicle_data, ssize_t* buffer_len)
{   
    
    struct capn c;
    capn_init_malloc(&c);
    capn_ptr cr = capn_root(&c);
    struct capn_segment* cs = cr.seg;
    VehicleData_ptr ptr = new_VehicleData(cs);
    write_VehicleData(&vehicle_state_data, ptr);
    int setp_ret = capn_setp(capn_root(&c), 0, ptr.p);
    //ASSERT_EQ(0, setp_ret);
    
    *buffer_len = capn_write_mem(&c, serialized_vehicle_data, 4096, 0 /* packed */);

    // hexDump("vehicle data",serialized_vehicle_data ,64,16);
    

    capn_free(&c);


}