#ifndef CAPN_FED348F2AC943A04
#define CAPN_FED348F2AC943A04
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

struct VehicleData;

typedef struct {capn_ptr p;} VehicleData_ptr;

typedef struct {capn_ptr p;} VehicleData_list;

struct VehicleData {
	unsigned isEmergency : 1;
	unsigned isHydrogenLeaking : 1;
	unsigned isScRelayClosed : 1;
	unsigned vehicleIsSpeedButtonPressed : 1;
	unsigned vehicleIsHalfSpeedButtonPressed : 1;
	unsigned hydrogenCellOneButtonState : 1;
	unsigned hydrogenCellTwoButtonState : 1;
	unsigned isSuperCapacitorButtonPressed : 1;
	int8_t logicState;
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
};

static const size_t VehicleData_word_count = 6;

static const size_t VehicleData_pointer_count = 0;

static const size_t VehicleData_struct_bytes_count = 48;


VehicleData_ptr new_VehicleData(struct capn_segment*);

VehicleData_list new_VehicleData_list(struct capn_segment*, int len);

void read_VehicleData(struct VehicleData*, VehicleData_ptr);

void write_VehicleData(const struct VehicleData*, VehicleData_ptr);

void get_VehicleData(struct VehicleData*, VehicleData_list, int i);

void set_VehicleData(const struct VehicleData*, VehicleData_list, int i);

#ifdef __cplusplus
}
#endif
#endif
