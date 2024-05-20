#ifndef CAPN_E662AE37D8C21445
#define CAPN_E662AE37D8C21445
/* AUTO GENERATED - DO NOT EDIT */
#include <capnp_c.h>

#if CAPN_VERSION != 1
#error "version mismatch between capnp_c.h and generated code"
#endif

#ifndef capnp_nowarn
# ifdef __GNUC__
#  define capnp_nowarn __extension__
# else
#  define capnp_nowarn
# endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct MTData;

typedef struct {capn_ptr p;} MTData_ptr;

typedef struct {capn_ptr p;} MTData_list;

enum MTData_FuelCellMode {
	MTData_FuelCellMode_noneSelected = 0,
	MTData_FuelCellMode_off = 1,
	MTData_FuelCellMode_prepareToRace = 2,
	MTData_FuelCellMode_race = 3
};

struct MTData {
	unsigned isEmergency : 1;
	unsigned isEmergencyButtonPressed : 1;
	unsigned isEmergencySwitchToggled : 1;
	unsigned isHydrogenLeaking : 1;
	unsigned isScRelayClosed : 1;
	unsigned isTimeResetButtonPressed : 1;
	unsigned isHalfSpeedButtonPressed : 1;
	unsigned isGasButtonPressed : 1;
	enum MTData_FuelCellMode fuelCellMode;
	float fcCurrent;
	float fcScCurrent;
	float scMotorCurrent;
	float fcVoltage;
	float scVoltage;
	float hydrogenSensorVoltage;
	float fuelCellTemperature;
	int32_t fanRpm;
	float vehicleSpeed;
	int32_t motorPwm;
	float hydrogenPressure;
	uint8_t lapNumber;
};

static const size_t MTData_word_count = 6;

static const size_t MTData_pointer_count = 0;

static const size_t MTData_struct_bytes_count = 48;


MTData_ptr new_MTData(struct capn_segment*);

MTData_list new_MTData_list(struct capn_segment*, int len);

void read_MTData(struct MTData*, MTData_ptr);

void write_MTData(const struct MTData*, MTData_ptr);

void get_MTData(struct MTData*, MTData_list, int i);

void set_MTData(const struct MTData*, MTData_list, int i);

#ifdef __cplusplus
}
#endif
#endif
