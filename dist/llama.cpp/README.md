EXPECTED FOLDER STRUCTURE

Archives are not tracked by git. `scripts/fetch-deps.sh` downloads the one
matching the host OS and extracts it into `llama-b9265/`.

```bash
dist/llama.cpp/
├── llama-b9265
│   ├── libggml-base.so -> libggml-base.so.0
│   ├── libggml-base.so.0 -> libggml-base.so.0.12.0
│   ├── libggml-base.so.0.12.0
│   ├── libggml-cpu-*.so                  # CPU backends (alderlake ... zen4)
│   ├── libggml-rpc.so
│   ├── libggml.so -> libggml.so.0
│   ├── libggml.so.0 -> libggml.so.0.12.0
│   ├── libggml.so.0.12.0
│   ├── libggml-vulkan.so
│   ├── libllama-common.so -> libllama-common.so.0
│   ├── libllama-common.so.0 -> libllama-common.so.0.0.9265
│   ├── libllama-common.so.0.0.9265
│   ├── libllama.so -> libllama.so.0
│   ├── libllama.so.0 -> libllama.so.0.0.9265
│   ├── libllama.so.0.0.9265
│   ├── libmtmd.so -> libmtmd.so.0
│   ├── libmtmd.so.0 -> libmtmd.so.0.0.9265
│   ├── libmtmd.so.0.0.9265
│   ├── LICENSE
│   ├── llama-batched-bench
│   ├── llama-bench
│   ├── llama-cli
│   ├── llama-completion
│   ├── llama-debug-template-parser
│   ├── llama-fit-params
│   ├── llama-gemma3-cli
│   ├── llama-gguf-split
│   ├── llama-imatrix
│   ├── llama-llava-cli
│   ├── llama-minicpmv-cli
│   ├── llama-mtmd-cli
│   ├── llama-mtmd-debug
│   ├── llama-perplexity
│   ├── llama-quantize
│   ├── llama-qwen2vl-cli
│   ├── llama-results
│   ├── llama-server
│   ├── llama-template-analysis
│   ├── llama-tokenize
│   ├── llama-tts
│   ├── models
│   │   └── Llama-3.2-1B-Instruct-Q4_K_M.gguf   # fetched by fetch-deps.sh
│   └── rpc-server
├── llama-b9265-bin-macos-arm64.tar.gz          # one per OS, gitignored,
├── llama-b9265-bin-ubuntu-vulkan-x64.tar.gz    # fetched by fetch-deps.sh
├── llama-b9265-bin-win-vulkan-x64.zip          # (Linux: ubuntu-vulkan)
├── llama-models.tar.gz
└── README.md
```
