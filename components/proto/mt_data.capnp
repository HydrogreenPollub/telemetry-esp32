@0xe662ae37d8c21445;

struct MTData {
    isEmergency @0 :Bool;
    isEmergencyButtonPressed @1 :Bool;
    isEmergencySwitchToggled @2 :Bool;
    isHydrogenLeaking @3 :Bool;
    isScRelayClosed @4 :Bool;
    isTimeResetButtonPressed @5 :Bool;
    isHalfSpeedButtonPressed @6 :Bool;
    isGasButtonPressed @7 :Bool;
    fuelCellMode @8 :FuelCellMode;

    fcCurrent @9 :Float32;
    fcScCurrent @10 :Float32;
    scMotorCurrent @11 :Float32;
    fcVoltage @12 :Float32;
    scVoltage @13 :Float32;
    hydrogenSensorVoltage @14 :Float32;
    fuelCellTemperature @15 :Float32;
    fanRpm @16 :Int32;
    vehicleSpeed @17 :Float32;
    motorPwm @18 :Int32;
    hydrogenPressure @19 :Float32;
    
    lapNumber @20 :UInt8;
    
    enum FuelCellMode {
    	noneSelected @0;
    	off @1;
    	prepareToRace @2;
    	race @3;
    }
}