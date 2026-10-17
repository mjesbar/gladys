EXPECTED FOLDER STRUCTURE

The model archive is not tracked by git. `scripts/fetch-deps.sh` downloads
`sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8.tar.bz2` and extracts it here.

```bash
dist/sherpaonnx/
├── sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8
│   ├── decoder.int8.onnx        # ~12 MB
│   ├── encoder.int8.onnx        # ~650 MB
│   ├── joiner.int8.onnx         # ~6 MB
│   ├── test_wavs/               # sample audio, not used by the app
│   └── tokens.txt
└── sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8.tar.bz2   # gitignored
```
