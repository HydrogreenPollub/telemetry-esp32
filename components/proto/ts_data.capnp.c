#include "ts_data.capnp.h"
/* AUTO GENERATED - DO NOT EDIT */
#ifdef __GNUC__
# define capnp_unused __attribute__((unused))
# define capnp_use(x) (void) x;
#else
# define capnp_unused
# define capnp_use(x)
#endif


TSData_ptr new_TSData(struct capn_segment *s) {
	TSData_ptr p;
	p.p = capn_new_struct(s, 88, 0);
	return p;
}
TSData_list new_TSData_list(struct capn_segment *s, int len) {
	TSData_list p;
	p.p = capn_new_list(s, len, 88, 0);
	return p;
}
void read_TSData(struct TSData *s capnp_unused, TSData_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->isEmergency = (capn_read8(p.p, 0) & 1) != 0;
	s->isEmergencyButtonPressed = (capn_read8(p.p, 0) & 2) != 0;
	s->isEmergencySwitchToggled = (capn_read8(p.p, 0) & 4) != 0;
	s->isHydrogenLeaking = (capn_read8(p.p, 0) & 8) != 0;
	s->isScRelayClosed = (capn_read8(p.p, 0) & 16) != 0;
	s->isTimeResetButtonPressed = (capn_read8(p.p, 0) & 32) != 0;
	s->isHalfSpeedButtonPressed = (capn_read8(p.p, 0) & 64) != 0;
	s->isGasButtonPressed = (capn_read8(p.p, 0) & 128) != 0;
	s->fuelCellMode = (enum TSData_FuelCellMode)(int) capn_read16(p.p, 2);
	s->fcCurrent = capn_to_f32(capn_read32(p.p, 4));
	s->fcScCurrent = capn_to_f32(capn_read32(p.p, 8));
	s->scMotorCurrent = capn_to_f32(capn_read32(p.p, 12));
	s->fcVoltage = capn_to_f32(capn_read32(p.p, 16));
	s->scVoltage = capn_to_f32(capn_read32(p.p, 20));
	s->hydrogenSensorVoltage = capn_to_f32(capn_read32(p.p, 24));
	s->fuelCellTemperature = capn_to_f32(capn_read32(p.p, 28));
	s->fanRpm = (int32_t) ((int32_t)capn_read32(p.p, 32));
	s->vehicleSpeed = capn_to_f32(capn_read32(p.p, 36));
	s->motorPwm = (int32_t) ((int32_t)capn_read32(p.p, 40));
	s->hydrogenPressure = capn_to_f32(capn_read32(p.p, 44));
	s->gpsLatitude = capn_to_f32(capn_read32(p.p, 48));
	s->gpsLongitude = capn_to_f32(capn_read32(p.p, 52));
	s->gpsAltitude = capn_to_f32(capn_read32(p.p, 56));
	s->gpsSpeed = capn_to_f32(capn_read32(p.p, 60));
	s->motorSpeed = capn_to_f32(capn_read32(p.p, 64));
	s->motorCurrent = capn_to_f32(capn_read32(p.p, 68));
	s->fcCurrentRaw = capn_to_f32(capn_read32(p.p, 72));
	s->fcVoltageRaw = capn_to_f32(capn_read32(p.p, 76));
	s->mcCurrent = capn_to_f32(capn_read32(p.p, 80));
	s->lapNumber = capn_read8(p.p, 1);
}
void write_TSData(const struct TSData *s capnp_unused, TSData_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write1(p.p, 0, s->isEmergency != 0);
	capn_write1(p.p, 1, s->isEmergencyButtonPressed != 0);
	capn_write1(p.p, 2, s->isEmergencySwitchToggled != 0);
	capn_write1(p.p, 3, s->isHydrogenLeaking != 0);
	capn_write1(p.p, 4, s->isScRelayClosed != 0);
	capn_write1(p.p, 5, s->isTimeResetButtonPressed != 0);
	capn_write1(p.p, 6, s->isHalfSpeedButtonPressed != 0);
	capn_write1(p.p, 7, s->isGasButtonPressed != 0);
	capn_write16(p.p, 2, (uint16_t) (s->fuelCellMode));
	capn_write32(p.p, 4, capn_from_f32(s->fcCurrent));
	capn_write32(p.p, 8, capn_from_f32(s->fcScCurrent));
	capn_write32(p.p, 12, capn_from_f32(s->scMotorCurrent));
	capn_write32(p.p, 16, capn_from_f32(s->fcVoltage));
	capn_write32(p.p, 20, capn_from_f32(s->scVoltage));
	capn_write32(p.p, 24, capn_from_f32(s->hydrogenSensorVoltage));
	capn_write32(p.p, 28, capn_from_f32(s->fuelCellTemperature));
	capn_write32(p.p, 32, (uint32_t) (s->fanRpm));
	capn_write32(p.p, 36, capn_from_f32(s->vehicleSpeed));
	capn_write32(p.p, 40, (uint32_t) (s->motorPwm));
	capn_write32(p.p, 44, capn_from_f32(s->hydrogenPressure));
	capn_write32(p.p, 48, capn_from_f32(s->gpsLatitude));
	capn_write32(p.p, 52, capn_from_f32(s->gpsLongitude));
	capn_write32(p.p, 56, capn_from_f32(s->gpsAltitude));
	capn_write32(p.p, 60, capn_from_f32(s->gpsSpeed));
	capn_write32(p.p, 64, capn_from_f32(s->motorSpeed));
	capn_write32(p.p, 68, capn_from_f32(s->motorCurrent));
	capn_write32(p.p, 72, capn_from_f32(s->fcCurrentRaw));
	capn_write32(p.p, 76, capn_from_f32(s->fcVoltageRaw));
	capn_write32(p.p, 80, capn_from_f32(s->mcCurrent));
	capn_write8(p.p, 1, s->lapNumber);
}
void get_TSData(struct TSData *s, TSData_list l, int i) {
	TSData_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_TSData(s, p);
}
void set_TSData(const struct TSData *s, TSData_list l, int i) {
	TSData_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_TSData(s, p);
}
