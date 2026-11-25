# ARM64 parity tests

These tests compare the original implementation and the ARM64 implementation
with identical deterministic inputs. The original objects are compiled with
renamed symbols so both variants can be linked into one test executable.

## Coverage

The ARM64 sources with independent implementations are covered function by
function:

| Source | Function | Comparison |
| --- | --- | --- |
| `Amp_compute.c` | `Clipping` | boundary values, infinities, exact result |
| `Amp_compute.c` | `GetPowerLevel` | negative, zero, scalar-tail and vector lengths |
| `Amp_compute.c` | `GetPowerEnergy` | negative, zero, scalar-tail and vector lengths |
| `Amp_compute.c` | `GetPowerdB` | negative, zero, scalar-tail and vector lengths |
| `Amp_compute.c` | `GetPowerdBShort` | negative, zero, scalar-tail and vector lengths |
| `Amp_compute.c` | `ChangeDbToWebExLevel` | every threshold interval |
| `Amp_compute.c` | `ShortToFloat` | PCM limits and deterministic random input |
| `Amp_compute.c` | `FloatToShort` | clipping, PCM output and mutated input |
| `rnn.c` | `compute_dense` | random weights/input, vector tail |
| `rnn.c` | `compute_gru` | random weights/input/state, vector tail |
| `rnn.c` | `compute_rnn` | 12 stateful frames and all nested state/output |
| `rnn.c` | `tansig_approx` | indirectly covered by TANH and SIGMOID layers |
| `rnn.c` | `sigmoid_approx` | indirectly covered by both output layers |
| `rnn.c` | `relu` | indirectly covered by the noise GRU |
| `rnn.c` | `activate` | indirectly covered with all three activation modes |
| `rnn.c` | `load_weights4` | indirectly covered by vectorized dense/GRU paths |
| `rnn.c` | `compute_gru_gate` | indirectly covered for all three GRU gates |
| `denoise.c` | `compute_band_energy` | all 481 complex bins and 22 bands |
| `denoise.c` | `compute_band_corr` | all 481 complex-bin pairs and 22 bands |
| `denoise.c` | `interp_band_gain` | all interpolated bins, including untouched tail behavior |
| `denoise.c` | `pitch_filter` | complex multiply-add, normalization and scalar tails |
| `celt_lpc.c` | `celt_fir` | 24-tap FIR over vector and scalar-tail output lengths |
| `celt_lpc.c` | `_celt_autocorr` | windowed input, 49 lags and tail dot products |
| `src_linear.c` | linear interpolation | 1, 2, 4 and 8 channels across stateful calls |
| `DCRemover.cpp` | constructor | paired object construction |
| `DCRemover.cpp` | `PrePosAnaInit` | exercised by each constructor |
| `DCRemover.cpp` | `findLevelAndDcRemove` | 16 frames at five vector/tail sizes |
| `DCRemover.cpp` | destructor | paired scope-exit destruction |
| `PSDsmooth.cpp` | frequency smoothing | low-band filter and symmetric constant-Q windows |
| `PSDsmooth.cpp` | alpha update | optimal smoothing mode across stateful frames |
| `PSDsmooth.cpp` | temporal smoothing | vector and scalar-tail transformed bands |
| `postfilter.cpp` | `UpdatePwr` | three complex spectra and stateful power smoothing |
| `postfilter.cpp` | `UpdateReferPwr` | complex reference power and all-band reduction |
| `postfilter.cpp` | `Spe_Limiter` | low-level decision and gain scaling |
| `postfilter.cpp` | frequency gain | offset/VAD zeroing and complex-bin gain |
| `AdapFilterGroup.cpp` | `filter` | complex FIR dot products with scalar tails |
| `AdapFilterGroup.cpp` | `UpdateFilterWeight` | complex tap updates across stateful frames |
| `AdapFilterGroup.cpp` | `SumR11_R12` | real and complex correlation reductions |
| `AdapFilterGroup.cpp` | full `process` | 48 deterministic stateful frames |

`SPEst` is retained as a stateful C++ regression test. Its ARM64 source directly
includes the original implementation.

All other production ARM64 sources are compatibility wrappers or independent
implementations. `check_source_mapping.sh` checks all 48 original-to-ARM64
production mappings in both directions. Wrapper files must include the original
source; independent implementations are accepted when they contain the ARM64
optimization guard.

`fft1536.c` remains a compatibility wrapper. Clang already vectorizes its
contiguous copy, sign, scale, column-DFT and reorder loops at width four. The
remaining non-vectorized loops are FFT control flow, bit-reversal scatters or
mixed-index butterflies; they are not replaced without a benchmark-positive
algorithm-level FFT implementation.

## Output

Each runtime comparison prints maximum absolute and scaled relative error.
Tests fail when the configured tolerance is exceeded. Integer and PCM outputs
require exact equality.

## Run

Use an ARM64 compiler:

```sh
make TARGET_ARCH=arm64 arm64-function-parity-test
```

On macOS ARM64, omit `arm64-source-check` because the legacy sample-rate
converter selects obsolete PowerPC inline assembly for every Apple target:

```sh
make TARGET_ARCH=arm64 \
  CC=/usr/bin/clang CXX=/usr/bin/clang++ AR=/usr/bin/ar \
  DEFINES=-fPIC \
  ARM64_OPT_FLAGS='-arch arm64 -ffp-contract=fast -fvectorize -fslp-vectorize -DAPF_ARM64_OPTIMIZED' \
  arm64-parity-test arm64-cpp-parity-test
./../../libs/arm64_parity_test
./../../libs/arm64_cpp_parity_test
sh arm64/tests/check_source_mapping.sh .
```
