from __future__ import annotations

import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASELINE = "a846929"
checks = 0


def fail(message):
    raise AssertionError(message)


def check(condition, message):
    global checks
    checks += 1
    if not condition:
        fail(message)


def git_text(revision_path):
    return subprocess.check_output(
        ["git", "show", f"{BASELINE}:{revision_path}"],
        cwd=ROOT,
        encoding="utf-8",
        text=True,
    )


def extract_function(source, name):
    match = re.search(rf"\b{re.escape(name)}\s*\([^;{{}}]*?\)[^;{{}}]*\{{", source, flags=re.S)
    if not match:
        fail(f"function not found: {name}")
    start = source.rfind("\n", 0, match.start()) + 1
    brace = source.find("{", match.start())
    depth = 0
    state = "code"
    index = brace
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line_comment"
                index += 1
            elif char == "/" and following == "*":
                state = "block_comment"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1].replace("\r\n", "\n")
        elif state == "line_comment" and char == "\n":
            state = "code"
        elif state == "block_comment" and char == "*" and following == "/":
            state = "code"
            index += 1
        elif state in {"string", "char"}:
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        index += 1
    fail(f"unterminated function: {name}")


def check_protected_functions():
    protected = {
        "user/src/SubFun.c": [
            "TimerDelay12_5us",
            "StartCaiyangLeft",
            "StartCaiyangRight",
        ],
        "MyDriver/src/cap.c": [
            "CapGpioConfig",
            "CapTimerInit",
            "TimerCountStart",
            "TIM14_IRQHandler",
        ],
        "user/src/main.c": ["SysTick_Handler"],
        "MyDriver/src/usart.c": [
            "UsartGpioConfig",
            "UsartConfig",
            "UsartNvicConfig",
            "USART1_IRQHandler",
        ],
        "MyDriver/src/flash.c": ["WriteFlashAll"],
        "MyDriver/src/gpio.c": ["GpioConfig"],
        "MyDriver/src/tim.c": ["TIM16Delay_Init"],
        "MyDriver/src/adc.c": ["Adc_Init_NTC"],
        "MyDriver/src/pwm.c": ["PwmGpioConfig", "TIM2_PWM_Init", "TIM3_PWM_Init"],
        "MyDriver/src/IIC_LQTP.c": [
            "Delay_Us",
            "IIC_GPIO_SET_Init",
            "LQTP_IIC_GPIO_Init",
            "IICSendByte_S",
            "IICReceiveByte_S",
        ],
        "MyDriver/src/iwdg.c": ["IWDG_Init", "IWDG_Feed"],
    }
    for relative_path, names in protected.items():
        baseline = git_text(relative_path)
        current = (ROOT / relative_path).read_text(encoding="utf-8")
        for name in names:
            check(
                extract_function(current, name) == extract_function(baseline, name),
                f"protected hardware function changed: {relative_path}:{name}",
            )


def check_vendor_unchanged():
    output = subprocess.check_output(
        [
            "git", "diff", "--name-only", BASELINE, "--",
            "CMSIS", "HK32F0xxA_StdPeriphDriver",
        ],
        cwd=ROOT,
        encoding="utf-8",
        text=True,
    ).strip()
    check(not output, f"vendor files changed: {output}")


def check_project():
    project_path = ROOT / "7500HA-YY.uvprojx"
    tree = ET.parse(project_path)
    target = tree.find("./Targets/Target")
    check(target is not None, "Keil target missing")
    common = target.find("./TargetOption/TargetCommonOption")
    check(common.findtext("Device") == "HK32F030K6TxA", "MCU changed")
    check("IROM(0x08000000,0x00010000)" in common.findtext("Cpu"), "IROM changed")
    check("IRAM(0x20000000,0x00002800)" in common.findtext("Cpu"), "IRAM changed")
    controls = target.find("./TargetOption/TargetArmAds/Cads/VariousControls")
    check(controls.findtext("Define") == "USE_STDPERIPH_DRIVER,HK32F030xxA", "defines changed")

    file_paths = []
    for node in target.findall("./Groups/Group/Files/File/FilePath"):
        relative = node.text.replace(".\\", "").replace("\\", "/")
        file_paths.append(relative)
        check((ROOT / relative).is_file(), f"Keil source missing: {relative}")
    check(len(file_paths) == len(set(path.lower() for path in file_paths)), "duplicate Keil source entry")
    for expected in [
        "user/src/measurement_core.c",
        "MyDriver/src/protocol_codec.c",
        "MyDriver/src/persistent_codec.c",
        "CoeffCountMod/CoeffCountMod V3.0 USF.lib",
        "CMSIS/HK32F0xxA/Source/ARM/startup_hk32f030xxa.s",
    ]:
        check(expected in file_paths, f"required Keil member missing: {expected}")


def define_value(source, name):
    match = re.search(rf"^\s*#define\s+{re.escape(name)}\s+([^/\r\n]+)", source, flags=re.M)
    if not match:
        fail(f"define missing: {name}")
    return match.group(1).strip()


def check_layout_and_periods():
    layout = (ROOT / "MyDriver/inc/persistent_layout.h").read_text(encoding="utf-8")
    expected = {
        "PERSISTENT_FLASH_PAGE_SIZE": "0x00000400UL",
        "PERSISTENT_FLASH_BASE": "0x08000000UL",
        "PERSISTENT_PRIMARY_PAGE_INDEX": "31UL",
        "PERSISTENT_BACKUP_PAGE_INDEX": "30UL",
        "PERSISTENT_DATA_SIZE": "260U",
        "PERSISTENT_CHECK_OFFSET": "258U",
        "PERSISTENT_ADDR_O2_POINTS": "0U",
        "PERSISTENT_ADDR_FLOW_POINTS": "46U",
        "PERSISTENT_ADDR_FLOW_ANCHORS": "60U",
        "PERSISTENT_ADDR_SERIAL_NUMBER": "120U",
        "PERSISTENT_ADDR_SEND_ORDER": "130U",
        "PERSISTENT_ADDR_WARN_VALUES": "131U",
        "PERSISTENT_ADDR_AUTO_SEND": "139U",
        "PERSISTENT_ADDR_O2_K": "140U",
        "PERSISTENT_ADDR_O2_B": "142U",
        "PERSISTENT_ADDR_AIR_TEMP_POINTS": "150U",
        "PERSISTENT_ADDR_N2_ANCHORS": "200U",
        "PERSISTENT_ADDR_BAUD_RATE": "255U",
    }
    for name, value in expected.items():
        check(define_value(layout, name) == value, f"persistent layout changed: {name}")

    systick = extract_function((ROOT / "user/src/main.c").read_text(encoding="utf-8"), "SysTick_Handler")
    thresholds = [int(value) for value in re.findall(r"Cnt(?:10|20|100|500|1000)ms\s*>=\s*(\d+)", systick)]
    check(thresholds == [10, 100, 100, 500, 1000], f"task thresholds changed: {thresholds}")

    protocol = (ROOT / "MyDriver/inc/RXExecute.h").read_text(encoding="utf-8")
    check(define_value(protocol, "ACK_RX").lower() == "0x16", "ACK byte changed")
    check(define_value(protocol, "NACK_RX").lower() == "0x06", "NACK byte changed")
    check(define_value(protocol, "LengthError").lower() == "0x01", "length error code changed")
    check(define_value(protocol, "NotCommand").lower() == "0x02", "unknown command code changed")
    check(define_value(protocol, "SENSOR_VERSION") == "YY", "sensor protocol variant changed")

    main_body = extract_function((ROOT / "user/src/main.c").read_text(encoding="utf-8"), "main")
    init_markers = [
        "SystemInit();",
        "SysTick_Config(",
        "GpioConfig();",
        "ReadEeprom(PERSISTENT_ADDR_BAUD_RATE)",
        "UsartInit(",
        "TIM16Delay_Init();",
        "Adc_Init_NTC();",
        "PwmInit();",
        "CapInit();",
        "FnRecover(&flash_data);",
        "LQTP_GPIO_Init();",
        "FnLinearization1();",
        "FnLinearization(1);",
        "sensor_value.temperature_NTC = test_tab();",
        "AotuSetSendOrder(",
        "IWDG_Init(4, 800);",
    ]
    positions = [main_body.find(marker) for marker in init_markers]
    check(all(position >= 0 for position in positions), "initialization call missing")
    check(positions == sorted(positions), "initialization order changed")


def check_forbidden_features():
    production = "\n".join(
        path.read_text(encoding="utf-8", errors="ignore")
        for folder in [ROOT / "user", ROOT / "MyDriver"]
        for path in folder.rglob("*.c")
    )
    for pattern, label in [
        (r"\b(?:malloc|calloc|realloc|free)\s*\(", "dynamic allocation"),
        (r"\b(?:osThread|xTaskCreate|vTaskDelay|FreeRTOS)\b", "RTOS"),
    ]:
        check(not re.search(pattern, production), f"forbidden feature introduced: {label}")


def main():
    check_protected_functions()
    check_vendor_unchanged()
    check_project()
    check_layout_and_periods()
    check_forbidden_features()
    print(f"PASS: {checks} static contract checks")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise
