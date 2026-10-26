from pathlib import Path
import re
import subprocess

from c_ast_runtime import CModule


ROOT = Path(__file__).resolve().parents[2]
MODULE = CModule(
    [
        ROOT / "user/src/measurement_core.c",
        ROOT / "MyDriver/src/protocol_codec.c",
        ROOT / "MyDriver/src/persistent_codec.c",
    ]
)

tests_run = 0


def baseline_function(source, name):
    match = re.search(rf"\b{re.escape(name)}\s*\([^;]*?\)\s*\{{", source, flags=re.S)
    if not match:
        raise AssertionError(f"baseline function not found: {name}")
    start = source.rfind("\n", 0, match.start()) + 1
    brace = source.find("{", match.start())
    depth = 0
    index = brace
    while index < len(source):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated baseline function: {name}")


def load_baseline_filters():
    source = subprocess.check_output(
        ["git", "show", "a846929:user/src/ResultCount.c"],
        cwd=ROOT,
        text=True,
        encoding="utf-8",
    )
    names = [
        "nOrderBuffer_5",
        "nOrderBuffer_10",
        "FilterFloatDataTask",
        "FilterIntDataTask",
        "AveFilterTask",
        "GetAverage",
    ]
    prefix = "typedef unsigned char uint8_t; typedef unsigned short uint16_t; typedef unsigned int uint32_t;\n"
    return CModule.from_source(prefix + "\n".join(baseline_function(source, name) for name in names))


def check(name, actual, expected, tolerance=None):
    global tests_run
    tests_run += 1
    if tolerance is None:
        passed = actual == expected
    else:
        passed = abs(actual - expected) <= tolerance
    if not passed:
        raise AssertionError(f"{name}: expected {expected!r}, got {actual!r}")


def test_protocol():
    request = [0x11, 0x01, 0x01, 0xED]
    check("valid checksum", MODULE.call("Protocol_IsChecksumValid", request, len(request)), 1)
    request[-1] ^= 1
    check("invalid checksum", MODULE.call("Protocol_IsChecksumValid", request, len(request)), 0)

    body = [0x11, 0x01, 0x01]
    check("checksum byte", MODULE.call("Protocol_ChecksumByte", body, len(body)), 0xED)

    length_error = [0x11, 0x03, 0x01, 0xAA, 0xBB, 0]
    MODULE.call("Protocol_EncodeLengthError", length_error)
    check("length error body", length_error[:4], [0x06, 0x02, 0x01, 0x01])
    check("length error checksum", MODULE.call("Protocol_ChecksumByte", length_error, 4), 0xF6)

    frame = [0] * 16
    length = MODULE.call("Protocol_EncodeLiveDataYY", frame, 500, 123, 500, 2500)
    check("YY live length", length, 11)
    check("YY live bytes", frame[:length], [0x16, 9, 1, 1, 244, 0, 123, 0, 250, 0, 0])
    check("YY live checksum", MODULE.call("Protocol_ChecksumByte", frame, length), 0x76)

    low_flow = [0] * 16
    MODULE.call("Protocol_EncodeLiveDataYY", low_flow, 900, 1, 240, 2500)
    check("low-flow air override", low_flow[3:5], [0x00, 0xD2])

    limited = [0] * 16
    MODULE.call("Protocol_EncodeLiveDataYY", limited, 2000, -2, 500, 2500)
    check("protocol O2 upper limit", limited[3:5], [0x03, 0xBC])
    check("protocol flow lower limit", limited[5:7], [0, 0])


def test_persistent_codec_and_defaults():
    data = [0] * 4
    MODULE.call("Persistent_WriteInt16", data, -3417)
    check("int16 little endian", data[:2], [0xA7, 0xF2])
    check("uint16 read", MODULE.call("Persistent_ReadUInt16", data), 0xF2A7)

    data = [0] * 4
    MODULE.call("Persistent_WriteLong32", data, 0x12345678)
    check("long little endian", data, [0x78, 0x56, 0x34, 0x12])
    check("long read", MODULE.call("Persistent_ReadLong32", data), 0x12345678)
    check("negative long read", MODULE.call("Persistent_ReadLong32", [0xFF] * 4), -1)

    check("warning valid", MODULE.call("Persistent_NormalizeWarning", 0, 700), 700)
    check("warning default 0", MODULE.call("Persistent_NormalizeWarning", 0, 0xFFFF), 700)
    check("warning default 1", MODULE.call("Persistent_NormalizeWarning", 1, 0xFFFF), 400)
    check("warning default 2", MODULE.call("Persistent_NormalizeWarning", 2, 0xFFFF), 500)
    check("O2 coefficient valid", MODULE.call("Persistent_IsO2CalibrationInvalid", 1142), 0)
    check("O2 coefficient invalid", MODULE.call("Persistent_IsO2CalibrationInvalid", 0xFFFF), 1)


def test_filters_and_windows():
    baseline = load_baseline_filters()
    check("median 5", MODULE.call("Measurement_Median5", [9, 1, 5, 3, 7]), 5)
    check("median 10", MODULE.call("Measurement_Median10", [10, 9, 8, 7, 6, 5, 4, 3, 2, 1]), 5)
    check("trimmed integer sum", MODULE.call("Measurement_TrimmedSumInt", [1, 2, 3, 4, 100], 5, 1), 9)
    check("trimmed float mean", MODULE.call("Measurement_TrimmedMeanFloat", [1.0, 2.0, 3.0, 4.0, 100.0], 5, 1), 3.0, 1e-6)
    check("average", MODULE.call("Measurement_AverageInt", [10, 20, 30, 40], 4), 25)

    vectors5 = [[9, 1, 5, 3, 7], [1, 1, 1, 1, 100], [-5, -1, -9, 7, 3]]
    for index, values in enumerate(vectors5):
        check(
            f"baseline median5 {index}",
            MODULE.call("Measurement_Median5", values),
            baseline.call("nOrderBuffer_5", values),
        )
    vectors10 = [list(range(10, 0, -1)), [5] * 10, [-5, 100, 2, 3, 4, 5, 6, 7, 8, 9]]
    for index, values in enumerate(vectors10):
        check(
            f"baseline median10 {index}",
            MODULE.call("Measurement_Median10", values),
            baseline.call("nOrderBuffer_10", values),
        )
    integer_values = [1, 2, 3, 4, 5, 6, 7, 100]
    check(
        "baseline trimmed integer",
        MODULE.call("Measurement_TrimmedSumInt", integer_values, 8, 2),
        baseline.call("FilterIntDataTask", integer_values, 8, 2),
    )
    float_values = [1.0, 2.0, 3.0, 4.0, 5.0, 100.0]
    check(
        "baseline trimmed float",
        MODULE.call("Measurement_TrimmedMeanFloat", float_values, 6, 1),
        baseline.call("FilterFloatDataTask", float_values, 6, 1),
        1e-6,
    )
    check(
        "baseline average",
        MODULE.call("Measurement_AverageInt", integer_values, len(integer_values)),
        baseline.call("GetAverage", integer_values, len(integer_values)),
    )

    check("window startup", MODULE.call("Measurement_SelectWindowOutput", 0, 12, 300, 250, 0), 300)
    check("window stable", MODULE.call("Measurement_SelectWindowOutput", 12, 12, 300, 250, 0), 250)
    check("window abnormal", MODULE.call("Measurement_SelectWindowOutput", 12, 12, 300, 250, 1), 300)
    check("window startup count", MODULE.call("Measurement_NextWindowCount", 0, 12, 0), 1)
    check("window stable count", MODULE.call("Measurement_NextWindowCount", 12, 12, 0), 12)
    check("window reset count", MODULE.call("Measurement_NextWindowCount", 12, 12, 1), 0)
    check("O2 startup output", MODULE.call("Measurement_ApplyO2Startup", 11, 12, 800), 209)
    check("O2 filled output", MODULE.call("Measurement_ApplyO2Startup", 12, 12, 800), 800)


def test_o2_flow_and_pwm():
    air = [0.0, 0.0, 1000.0]
    oxygen = [0.0, 0.0, 2000.0]
    air_value = MODULE.call("Measurement_CalculateO2", 500, 500, 2500, air, oxygen, 1000, 1000)
    oxygen_value = MODULE.call("Measurement_CalculateO2", 1000, 1000, 2500, air, oxygen, 1000, 1000)
    check("O2 air point", MODULE.call("Measurement_ClampO2", air_value), 209)
    check("O2 calibration point", MODULE.call("Measurement_ClampO2", oxygen_value), 1000)
    check("O2 lower limit", MODULE.call("Measurement_ClampO2", -5.0), 0)
    check("O2 upper limit", MODULE.call("Measurement_ClampO2", 1300.0), 1200)

    args = [10.0, 1.0, 4.0, 20.0, 2.0, 3.0, 30.0, 3.0, 2.0, 40.0, 4.0, 1.0, 50.0, 5.0]
    check("flow segment primary", MODULE.call("Measurement_ApplyFlowSegments", 5.0, *args), 51.0, 1e-6)
    check("flow boundary x1 strict", MODULE.call("Measurement_ApplyFlowSegments", 4.0, *args), 82.0, 1e-6)
    check("flow boundary x2 strict", MODULE.call("Measurement_ApplyFlowSegments", 3.0, *args), 93.0, 1e-6)
    check("flow boundary x3 strict", MODULE.call("Measurement_ApplyFlowSegments", 2.0, *args), 84.0, 1e-6)
    check("flow boundary x4 strict", MODULE.call("Measurement_ApplyFlowSegments", 1.0, *args), 55.0, 1e-6)
    check("flow round", MODULE.call("Measurement_ClampFlow", 12.34), 123)
    check("flow negative", MODULE.call("Measurement_ClampFlow", -1.0), 0)
    check("flow overrange", MODULE.call("Measurement_ClampFlow", 1000.0), 0)

    check("PWM lower", MODULE.call("Measurement_MapPwmDuty", -1), 0)
    check("PWM zero", MODULE.call("Measurement_MapPwmDuty", 0), 0)
    check("PWM midpoint", MODULE.call("Measurement_MapPwmDuty", 500), 382)
    check("PWM full scale", MODULE.call("Measurement_MapPwmDuty", 1000), 764)
    check("PWM upper", MODULE.call("Measurement_MapPwmDuty", 1200), 764)


if __name__ == "__main__":
    test_protocol()
    test_persistent_codec_and_defaults()
    test_filters_and_windows()
    test_o2_flow_and_pwm()
    print(f"PASS: {tests_run} characterization assertions")
