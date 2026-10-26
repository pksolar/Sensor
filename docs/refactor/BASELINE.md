# 重构基线

记录日期：2026-10-26（Asia/Shanghai）

## 恢复点

- 原目录在检查时不是 Git 仓库。
- 完整只读基线备份：`D:\project\7500HA-YY_backup_20261026-203647`
- 备份核对：271 个文件，37,266,134 字节；源目录与备份目录的文件数量、总字节数一致。
- 完整 SHA-256 清单：`D:\project\7500HA-YY_backup_20261026-203647\ORIGINAL_SHA256_MANIFEST.csv`（156 个源码、头文件、工程及产物条目）。
- 本地 Git 基线提交：`a846929c98653813d88705eca94e20e7eefdfb16`，分支 `refactor-baseline`。
- 恢复建议：不要覆盖当前目录。将完整备份复制到新的并列目录，或用 Git 查看/恢复单批源码变更；历史构建产物以外部备份为准。

## 关键原始哈希

| 文件 | 大小 | SHA-256 |
|---|---:|---|
| `7500HA-YY.uvprojx` | 21,104 | `0E9B2EBB4C474BD567A3C58B11C70ADAC29D3AAECD5C83EBD30A58DF8FAA7BF3` |
| `user/src/main.c` | 14,793 | `5529B8B99DE8A9C39198778060B64DE4F8703F913FDFDD0EC4FDD132E8B2A7E4` |
| `CoeffCountMod/CoeffCountMod V3.0 USF.lib` | 19,734 | `F9D903F3216093143E8011101083F575C5C38D302FF7BCA5309BE7D8BE368923` |
| `Objects/7500HA-YY.hex` | 70,476 | `398EABADAD6B3308701FD1A61E263001E7F1AF91672E82F4E01172904765BA77` |
| `Objects/7500HA-YY.bin` | 25,040 | `A71DA60E56B7256E4BD031CDD2EF6DB48A673A9DFE511363487420BFA8EB392D` |
| `Listings/7500HA-YY.map` | 224,182 | `107D2CD0115E86D320DFA47F1E7CDFB1EE1CD8C7C11763B0489E6FF20EA23BC7` |

完整原始构建日志及其哈希保存在外部备份。首次本机构建尝试会覆盖被 `.gitignore` 忽略的 `Objects/7500HA-YY.build_log.htm`，因此不能用当前该文件代替备份中的历史日志。

## 工程配置

- 目标：`hk32f030k6t6a`
- 器件：`HK32F030K6TxA`，Cortex-M0，小端
- IROM：`0x08000000`，长度 `0x10000`（64 KiB）
- IRAM：`0x20000000`，长度 `0x2800`（10 KiB）
- 宏：`USE_STDPERIPH_DRIVER,HK32F030xxA`
- C99：启用；优化等级项目值为 4；警告等级项目值为 2；使用 MicroLIB。
- include：标准外设库、CMSIS device、`user/inc`、CMSIS core、`MyDriver/inc`、`CoeffCountMod`。
- 输出：`Objects/7500HA-YY.axf`、HEX；后处理 `fromelf.exe --bin` 生成 BIN。
- 启动：`CMSIS/HK32F0xxA/Source/ARM/startup_hk32f030xxa.s`。
- 闭源库：`CoeffCountMod/CoeffCountMod V3.0 USF.lib`。

## 历史构建证据（仅参考）

备份中的日志记录：µVision 5.36，ARMCC 5.06 update 7 build 960，0 errors、2 warnings；警告均在 `LQTP.c`，为 `temp0`/`temp1` “set but never used”。历史 map：

- Code 24,036 B，RO-data 716 B，RW-data 288 B，ZI-data 3,416 B。
- Total RO 24,752 B；Total RW 3,704 B；Total ROM 25,040 B。
- 历史 ROM 低于 64 KiB，RAM 低于 10 KiB。

等级：`STATICALLY_REVIEWED`。这些产物来自旧路径 `D:\Initial_Repository\HK32\7500HA-YY`，不是本次环境重新构建结果。

## 本机构建能力

- 找到 µVision `D:\software\keil\UV4\UV4.exe`，版本 5.40.0.12。
- 找到 ARM Compiler 6.22（armclang），但未找到 ARMCC 5。
- 2026-10-26 本机重建退出码 2：目标要求 “Default Compiler Version 5”，该工具链不可用，重建中止。
- ARM Compiler 6 后续仅用于语法、链接和资源补充检查，不作为 ARMCC 5 替代验收。

等级：目标构建 `HARDWARE_VALIDATION_PENDING` 不适用；准确状态为“构建工具缺失，未获得 `BUILD_VERIFIED`”。
