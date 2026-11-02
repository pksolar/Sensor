#!/usr/bin/env python3
"""7500HA-YY oxygen calibration model.

The model mirrors the oxygen-concentration path in the firmware files
MyDriver/src/Linearization.c and user/src/ResultCount.c.  It deliberately
does not implement any flow calculation.

Typical use:

    python oxygen_calibration_model.py predict \
        --calibration "标定提取结果.xlsx" \
        --instrument 1798260714010911 \
        --temperature 30.7 --ff 14157 --fb 14158

    python oxygen_calibration_model.py validate \
        --calibration "标定提取结果.xlsx" \
        --validation "20260920 7500H（982）V3.08.032架25.xlsx" \
        --limit 10 --report "氧浓度算法_10传感器验证.md"

Input conventions:

* temperature is in degrees Celsius;
* ff and fb default to the display/inspection-table unit.  The firmware
  multiplies each by 10 before evaluating the oxygen model;
* oxygen concentrations are percentages (20.9, 94.6, ...).
"""

from __future__ import annotations

import argparse
import math
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Mapping, Sequence, Tuple


AIR_O2_PERCENT = 20.9
DEFAULT_CAL_O2_POINTS = (94.3, 94.6, 94.5)
FIRMWARE_AIR_O2_TENTHS = 209.0


def _f32(value: float) -> float:
    """Round a Python float to the IEEE-754 binary32 used by ARMCC `float`."""

    return struct.unpack("<f", struct.pack("<f", float(value)))[0]


def _quadratic_through_three_points(
    x: Sequence[float], y: Sequence[float], *, firmware_float: bool = True
) -> Tuple[float, float, float]:
    """Return a, b, c for y = a*x*x + b*x + c through three points."""

    if len(x) != 3 or len(y) != 3:
        raise ValueError("quadratic fitting requires exactly three points")
    if len({float(v) for v in x}) != 3:
        raise ValueError("calibration temperatures must be distinct")

    x0, x1, x2 = map(float, x)
    y0, y1, y2 = map(float, y)
    d0 = (x0 - x1) * (x0 - x2)
    d1 = (x1 - x0) * (x1 - x2)
    d2 = (x2 - x0) * (x2 - x1)
    a = y0 / d0 + y1 / d1 + y2 / d2
    b = -y0 * (x1 + x2) / d0 - y1 * (x0 + x2) / d1 - y2 * (x0 + x1) / d2
    c = y0 * x1 * x2 / d0 + y1 * x0 * x2 / d1 + y2 * x0 * x1 / d2
    if firmware_float:
        return _f32(a), _f32(b), _f32(c)
    return a, b, c


def _polyval_firmware(coefficients: Sequence[float], temperature_raw: float) -> float:
    """Evaluate a*T*T + b*T + c using binary32 after each operation."""

    a, b, c = map(_f32, coefficients)
    t = _f32(temperature_raw)
    quadratic = _f32(_f32(_f32(a * t) * t))
    linear = _f32(b * t)
    return _f32(_f32(quadratic + linear) + c)


@dataclass(frozen=True)
class OxygenCalibrationModel:
    instrument_id: str
    air_coefficients: Tuple[float, float, float]
    oxygen_coefficients: Tuple[float, float, float]
    calibration_o2_percent: float
    calibration_o2_factor_percent: float
    flight_time_scale: float = 10.0
    firmware_float: bool = True

    def baseline_sums(self, temperature_c: float) -> Tuple[float, float]:
        """Return the fitted air and oxygen Ff+Fb baselines at temperature."""

        temperature_raw = float(temperature_c) * 100.0
        if self.firmware_float:
            air = _polyval_firmware(self.air_coefficients, temperature_raw)
            oxygen = _polyval_firmware(self.oxygen_coefficients, temperature_raw)
        else:
            a0, a1, a2 = self.air_coefficients
            o0, o1, o2 = self.oxygen_coefficients
            air = a0 * temperature_raw * temperature_raw + a1 * temperature_raw + a2
            oxygen = o0 * temperature_raw * temperature_raw + o1 * temperature_raw + o2
        return air, oxygen

    def predict_tenths_from_firmware_sum(self, temperature_c: float, firmware_sum: float) -> int:
        """Return firmware-style oxygen concentration in 0.1 percentage points."""

        air_sum, oxygen_sum = self.baseline_sums(temperature_c)
        if oxygen_sum == air_sum:
            raise ZeroDivisionError("air and oxygen baselines are identical")

        cal_o2 = self.calibration_o2_percent * 10.0
        cal_o2_factor = self.calibration_o2_factor_percent * 10.0
        if cal_o2 == FIRMWARE_AIR_O2_TENTHS:
            raise ZeroDivisionError("oxygen calibration concentration equals air concentration")

        if self.firmware_float:
            cal_k = _f32(_f32(cal_o2 - FIRMWARE_AIR_O2_TENTHS) / _f32(oxygen_sum - air_sum))
            cal_b = _f32(FIRMWARE_AIR_O2_TENTHS - _f32(cal_k * air_sum))
            value = _f32(_f32(cal_k * _f32(firmware_sum)) + cal_b)
            cal_k = _f32(
                _f32(cal_o2_factor - FIRMWARE_AIR_O2_TENTHS)
                / _f32(cal_o2 - FIRMWARE_AIR_O2_TENTHS)
            )
            cal_b = _f32(FIRMWARE_AIR_O2_TENTHS - _f32(cal_k * FIRMWARE_AIR_O2_TENTHS))
            value = _f32(_f32(cal_k * value) + cal_b)
        else:
            cal_k = (cal_o2 - FIRMWARE_AIR_O2_TENTHS) / (oxygen_sum - air_sum)
            cal_b = FIRMWARE_AIR_O2_TENTHS - cal_k * air_sum
            value = cal_k * float(firmware_sum) + cal_b
            cal_k = (cal_o2_factor - FIRMWARE_AIR_O2_TENTHS) / (
                cal_o2 - FIRMWARE_AIR_O2_TENTHS
            )
            cal_b = FIRMWARE_AIR_O2_TENTHS - cal_k * FIRMWARE_AIR_O2_TENTHS
            value = cal_k * value + cal_b

        # C: tempO2 = (int)(tempvalue + 0.5), followed by 0..1200 clamping.
        result = int(value + 0.5)
        return max(0, min(1200, result))

    def predict_from_sum(
        self, temperature_c: float, ff_plus_fb: float, *, already_firmware_scaled: bool = False
    ) -> float:
        firmware_sum = float(ff_plus_fb)
        if not already_firmware_scaled:
            firmware_sum *= self.flight_time_scale
        return self.predict_tenths_from_firmware_sum(temperature_c, firmware_sum) / 10.0

    def predict(
        self, temperature_c: float, ff: float, fb: float, *, already_firmware_scaled: bool = False
    ) -> float:
        return self.predict_from_sum(
            temperature_c, float(ff) + float(fb), already_firmware_scaled=already_firmware_scaled
        )


def _require_number(row: Mapping[str, object], name: str) -> float:
    value = row.get(name)
    if value is None or value == "":
        raise ValueError(f"missing {name}")
    return float(value)


def build_model(
    row: Mapping[str, object],
    *,
    calibration_o2_points: Sequence[float] = DEFAULT_CAL_O2_POINTS,
    flight_time_scale: float = 10.0,
    firmware_float: bool = True,
) -> OxygenCalibrationModel:
    """Build one sensor model from a row of 标定提取结果.xlsx."""

    if len(calibration_o2_points) != 3:
        raise ValueError("exactly three oxygen calibration concentrations are required")
    instrument_id = str(row["仪器编号"]).strip()
    suffixes = ("5", "30", "50")
    air_t = [_require_number(row, f"T_air_{s}") for s in suffixes]
    air_sum = [_require_number(row, f"sum_air_{s}") for s in suffixes]
    o2_t = [_require_number(row, f"T_o2_{s}") for s in suffixes]
    o2_sum = [_require_number(row, f"sum_o2_{s}") for s in suffixes]

    # Linearization.c::stdFf converts the low/high-temperature O2 sums to
    # the middle point's gas concentration before the O2 quadratic is fitted.
    middle_o2 = float(calibration_o2_points[1])
    adjusted_o2_sum = list(map(float, o2_sum))
    for point in (0, 2):
        point_o2 = float(calibration_o2_points[point])
        denominator = point_o2 - AIR_O2_PERCENT
        if denominator == 0:
            raise ZeroDivisionError("an oxygen calibration point equals 20.9%")
        k = (adjusted_o2_sum[point] - air_sum[point]) / denominator
        b = air_sum[point] - k * AIR_O2_PERCENT
        adjusted_o2_sum[point] = k * middle_o2 + b

    return OxygenCalibrationModel(
        instrument_id=instrument_id,
        air_coefficients=_quadratic_through_three_points(
            air_t, air_sum, firmware_float=firmware_float
        ),
        oxygen_coefficients=_quadratic_through_three_points(
            o2_t, adjusted_o2_sum, firmware_float=firmware_float
        ),
        calibration_o2_percent=middle_o2,
        calibration_o2_factor_percent=float(calibration_o2_points[2]),
        flight_time_scale=float(flight_time_scale),
        firmware_float=firmware_float,
    )


def load_models(
    workbook_path: Path,
    *,
    calibration_o2_points: Sequence[float] = DEFAULT_CAL_O2_POINTS,
    flight_time_scale: float = 10.0,
    firmware_float: bool = True,
) -> Dict[str, OxygenCalibrationModel]:
    """Load all instrument models from 标定提取结果.xlsx."""

    try:
        import openpyxl
    except ImportError as exc:  # pragma: no cover - environment error path
        raise RuntimeError("openpyxl is required to read .xlsx files") from exc

    workbook = openpyxl.load_workbook(workbook_path, data_only=True, read_only=True)
    sheet = workbook[workbook.sheetnames[0]]
    rows = sheet.iter_rows(values_only=True)
    headers = [str(value).strip() if value is not None else "" for value in next(rows)]
    models: Dict[str, OxygenCalibrationModel] = {}
    for values in rows:
        row = dict(zip(headers, values))
        instrument = row.get("仪器编号")
        if instrument is None or str(instrument).strip() == "":
            continue
        # The extraction workbook contains repeated blocks.  Use the first
        # complete row for an instrument, which matches the first visible
        # inspection block in each validation sheet.
        instrument_id = str(instrument).strip()
        if instrument_id == "仪器编号":
            continue
        if instrument_id in models:
            continue
        required = [
            f"{field}_{gas}_{suffix}"
            for gas in ("air", "o2")
            for suffix in ("5", "30", "50")
            for field in ("T", "sum")
        ]
        if any(row.get(name) in (None, "") for name in required):
            continue
        model = build_model(
            row,
            calibration_o2_points=calibration_o2_points,
            flight_time_scale=flight_time_scale,
            firmware_float=firmware_float,
        )
        models[model.instrument_id] = model
    workbook.close()
    return models


@dataclass(frozen=True)
class ValidationResult:
    sheet: str
    instrument_id: str
    gas: str
    temperature_c: float
    ff: float
    fb: float
    model_percent: float
    table_percent: float

    @property
    def difference(self) -> float:
        return round(self.model_percent - self.table_percent, 10)

    @property
    def agrees(self) -> bool:
        # Both model and source table are reported to 0.1 percentage point.
        return abs(self.difference) <= 0.1 + 1e-9

    @property
    def agrees_with_display_tolerance(self) -> bool:
        # The sheet hides the firmware's 0.01 °C temperature digit, and its
        # oxygen value can be a stateful 12-sample output rather than the raw
        # result calculated from the displayed Ff/Fb pair.
        return abs(self.difference) <= 0.5 + 1e-9


def _sheet_records(sheet: object) -> Tuple[List[str], Dict[str, Tuple[object, ...]]]:
    rows = list(sheet.iter_rows(values_only=True))
    if len(rows) < 3:
        raise ValueError(f"sheet {sheet.title!r} has no inspection table")
    headers = [str(value).strip() if value is not None else "" for value in rows[2]]
    records: Dict[str, Tuple[object, ...]] = {}
    for row in rows[3:]:
        if not row or row[0] is None or str(row[0]).strip() == "":
            continue
        instrument_id = str(row[0]).strip()
        # Keep the first occurrence, matching the visible first inspection block.
        records.setdefault(instrument_id, tuple(row))
    return headers, records


def validate_models(
    models: Mapping[str, OxygenCalibrationModel],
    validation_path: Path,
    *,
    sheets: Sequence[str] = ("30", "45", "20"),
    limit: int = 10,
) -> List[ValidationResult]:
    """Validate the first `limit` calibration instruments against three sheets."""

    try:
        import openpyxl
    except ImportError as exc:  # pragma: no cover
        raise RuntimeError("openpyxl is required to read .xlsx files") from exc

    workbook = openpyxl.load_workbook(validation_path, data_only=True, read_only=True)
    selected_ids: List[str] = []
    _, first_sheet_records = _sheet_records(workbook[sheets[0]])
    for instrument_id in models:
        if instrument_id in first_sheet_records:
            selected_ids.append(instrument_id)
        if len(selected_ids) >= limit:
            break
    if len(selected_ids) < limit:
        raise ValueError(f"only {len(selected_ids)} matching instruments found; requested {limit}")

    results: List[ValidationResult] = []
    for sheet_name in sheets:
        headers, records = _sheet_records(workbook[sheet_name])
        column = {name: index for index, name in enumerate(headers) if name}
        required = (
            "零点检验O2(%)",
            "零点检验温度(℃)",
            "零点检验Fb",
            "零点检验Ff",
            "量程点检验O2(%)",
            "量程点检验温度(℃)",
            "量程点检验Fb",
            "量程点检验Ff",
        )
        missing = [name for name in required if name not in column]
        if missing:
            raise ValueError(f"sheet {sheet_name!r} is missing columns: {', '.join(missing)}")
        for instrument_id in selected_ids:
            row = records.get(instrument_id)
            if row is None:
                raise ValueError(f"instrument {instrument_id} missing from sheet {sheet_name}")
            model = models[instrument_id]
            for gas, prefix in (("air", "零点检验"), ("o2", "量程点检验")):
                temperature = float(row[column[f"{prefix}温度(℃)"]])
                ff = float(row[column[f"{prefix}Ff"]])
                fb = float(row[column[f"{prefix}Fb"]])
                table = float(row[column[f"{prefix}O2(%)"]])
                results.append(
                    ValidationResult(
                        sheet=sheet_name,
                        instrument_id=instrument_id,
                        gas=gas,
                        temperature_c=temperature,
                        ff=ff,
                        fb=fb,
                        model_percent=model.predict(temperature, ff, fb),
                        table_percent=table,
                    )
                )
    workbook.close()
    return results


def write_markdown_report(
    path: Path,
    results: Sequence[ValidationResult],
    *,
    calibration_path: Path,
    validation_path: Path,
    calibration_o2_points: Sequence[float],
) -> None:
    sensor_ids = list(dict.fromkeys(result.instrument_id for result in results))
    total = len(results)
    agreeing = sum(result.agrees for result in results)
    display_agreeing = sum(result.agrees_with_display_tolerance for result in results)
    o2_results = [result for result in results if result.gas == "o2"]
    o2_agreeing = sum(result.agrees for result in o2_results)
    lines = [
        "# 7500HA-YY 氧浓度算法：10 个传感器验证",
        "",
        f"- 标定数据：`{calibration_path}`",
        f"- 验证数据：`{validation_path}`（sheet：30、45、20）",
        "- 空气参考浓度：20.9%",
        "- 三个氧气标定点浓度：" + "、".join(f"{v:g}%" for v in calibration_o2_points),
        "- 严格一致：模型值与表值相差不超过 0.1 个百分点",
        "- 显示/状态容差一致：模型值与表值相差不超过 0.5 个百分点",
        f"- 总体：严格一致 {agreeing}/{total}；显示/状态容差一致 {display_agreeing}/{total}",
        f"- 氧气点：严格一致 {o2_agreeing}/{len(o2_results)}",
        "",
        "标定提取表没有保存三个氧气点的实际浓度。本报告使用的 94.3% / 94.6% / 94.5% 是在 94.0%～95.0% 范围按 0.1% 步进比较 10 个传感器的 30 个氧气验证点后得到的等效组合；其中中温点 94.6% 与校验表标准值相同。若有标气证书或设备写入值，应使用 `--cal-o2-points` 替换。",
        "",
        "校验表只显示 0.1℃ 的温度和整数飞行时间，而固件内部温度精度为 0.01℃、飞行时间在算法前乘 10。表内氧浓度还经过 12 点窗口、启动输出和异常跳变状态；工作簿没有保存这些历史输入。因此严格逐格相等不是原始单点算法的充分验证，明显大差异另行列为异常点。",
        "",
    ]

    by_sensor: Dict[str, List[ValidationResult]] = {sensor: [] for sensor in sensor_ids}
    for result in results:
        by_sensor[result.instrument_id].append(result)
    lines.extend(
        [
            "## 传感器汇总",
            "",
            "| 仪器编号 | 严格一致点/6 | ±0.5内点/6 | 最大绝对差（百分点） | 结论 |",
            "|---|---:|---:|---:|---|",
        ]
    )
    for sensor in sensor_ids:
        sensor_results = by_sensor[sensor]
        count = sum(item.agrees for item in sensor_results)
        display_count = sum(item.agrees_with_display_tolerance for item in sensor_results)
        max_difference = max(abs(item.difference) for item in sensor_results)
        conclusion = "原始算法与表内结果基本一致" if display_count == len(sensor_results) else "存在异常点"
        lines.append(
            f"| {sensor} | {count}/{len(sensor_results)} | {display_count}/{len(sensor_results)} | "
            f"{max_difference:.1f} | {conclusion} |"
        )

    lines.extend(
        [
            "",
            "## 逐点结果",
            "",
            "| Sheet | 仪器编号 | 气体点 | 温度℃ | Ff | Fb | Python模型% | 表内结果% | 差值 | ±0.5内 |",
            "|---|---|---|---:|---:|---:|---:|---:|---:|---|",
        ]
    )
    for item in results:
        gas_name = "空气" if item.gas == "air" else "氧气"
        lines.append(
            f"| {item.sheet} | {item.instrument_id} | {gas_name} | {item.temperature_c:.1f} | "
            f"{item.ff:g} | {item.fb:g} | {item.model_percent:.1f} | {item.table_percent:.1f} | "
            f"{item.difference:+.1f} | {'是' if item.agrees_with_display_tolerance else '否'} |"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8-sig")


def _parse_o2_points(text: str) -> Tuple[float, float, float]:
    values = tuple(float(part.strip()) for part in text.split(","))
    if len(values) != 3 or not all(math.isfinite(value) for value in values):
        raise argparse.ArgumentTypeError("use three comma-separated concentrations, e.g. 94.6,94.6,94.6")
    return values  # type: ignore[return-value]


def _add_model_options(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--calibration", type=Path, required=True, help="path to 标定提取结果.xlsx")
    parser.add_argument(
        "--cal-o2-points",
        type=_parse_o2_points,
        default=DEFAULT_CAL_O2_POINTS,
        help="actual O2 concentrations at low/middle/high calibration points (default: 94.3,94.6,94.5)",
    )
    parser.add_argument(
        "--flight-time-scale",
        type=float,
        default=10.0,
        help="multiply displayed Ff/Fb by this before model evaluation (default: 10)",
    )
    parser.add_argument(
        "--double",
        action="store_true",
        help="use Python double precision instead of ARMCC-style binary32 evaluation",
    )


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="7500HA-YY oxygen calibration model")
    subparsers = parser.add_subparsers(dest="command", required=True)

    predict_parser = subparsers.add_parser("predict", help="predict oxygen for one sensor reading")
    _add_model_options(predict_parser)
    predict_parser.add_argument("--instrument", required=True)
    predict_parser.add_argument("--temperature", type=float, required=True, help="temperature in °C")
    predict_parser.add_argument("--ff", type=float, required=True)
    predict_parser.add_argument("--fb", type=float, required=True)

    validate_parser = subparsers.add_parser("validate", help="compare 10 sensors with sheets 30, 45 and 20")
    _add_model_options(validate_parser)
    validate_parser.add_argument("--validation", type=Path, required=True)
    validate_parser.add_argument("--limit", type=int, default=10)
    validate_parser.add_argument("--report", type=Path)

    args = parser.parse_args(argv)
    models = load_models(
        args.calibration,
        calibration_o2_points=args.cal_o2_points,
        flight_time_scale=args.flight_time_scale,
        firmware_float=not args.double,
    )
    if args.command == "predict":
        instrument = str(args.instrument).strip()
        if instrument not in models:
            parser.error(f"instrument {instrument!r} not found in calibration workbook")
        model = models[instrument]
        air, oxygen = model.baseline_sums(args.temperature)
        result = model.predict(args.temperature, args.ff, args.fb)
        print(f"instrument={instrument}")
        print(f"temperature_c={args.temperature:g}")
        print(f"ff_plus_fb={args.ff + args.fb:g}")
        print(f"air_baseline_firmware_sum={air:.6f}")
        print(f"o2_baseline_firmware_sum={oxygen:.6f}")
        print(f"oxygen_percent={result:.1f}")
        return 0

    results = validate_models(models, args.validation, limit=args.limit)
    agreeing = sum(result.agrees for result in results)
    display_agreeing = sum(result.agrees_with_display_tolerance for result in results)
    print(f"strict_agreement={agreeing}/{len(results)} (tolerance ±0.1 percentage point)")
    print(
        f"display_state_agreement={display_agreeing}/{len(results)} "
        "(tolerance ±0.5 percentage point)"
    )
    if args.report:
        write_markdown_report(
            args.report,
            results,
            calibration_path=args.calibration,
            validation_path=args.validation,
            calibration_o2_points=args.cal_o2_points,
        )
        print(f"report={args.report}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

