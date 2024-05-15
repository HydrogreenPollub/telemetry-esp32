@0xfed348f2ac943a04;

struct TSData {
    isEmergency @0 :Bool;
    isHydrogenLeaking @1 :Bool;
    isScRelayClosed @2 :Bool;
    vehicleIsSpeedButtonPressed @3 :Bool;
    vehicleIsHalfSpeedButtonPressed @4 :Bool;
    hydrogenCellOneButtonState @5 :Bool;
    hydrogenCellTwoButtonState @6 :Bool;
    isSuperCapacitorButtonPressed @7 :Bool;
    logicState @8 :Int8;

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
    fcCurrentRaw @20 :Float32;
    fcVoltageRaw @21 :Float32;
    mcCurrent @22 :Float32;
}
