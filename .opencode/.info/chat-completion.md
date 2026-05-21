This is the approach to pipe audio files to the llama-server.

# Gemma

```bash
NERV🍁 ~/Github/mjesbar/gladys/bin/llama.cpp/llama-b9049 > time cat <<EOF | curl -s http://127.0.0.1:8080/v1/chat/completions   -H "Content-Type: application/json"   -d @- | jq '.choices[0].message.content'
{
  "messages": [
    {
      "role": "user",
      "content": [
        {"type": "text", "text": "Transcribe the audio into a Markdown list. Output ONLY the list."},
        {
          "type": "image_url",
          "image_url": {
            "url": "data:image/png;base64,$(base64 -w 0 samples/filename.wav)"
          }
        }
      ]
    }
  ],
  "temperature": 0.0
}
EOF
"* Primero, necesito ir a la tienda.\n* Segundo, necesito encontrar el producto que estoy buscando.\n* Tercero, necesito pagar.\n* Fin."

real    0m2.903s
user    0m0.017s
sys     0m0.016s
```

# Llama Ultravox

```bash
time cat <<EOF | curl -s http://127.0.0.1:8080/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d @- | jq '.choices[0].message.content'
{
  "messages": [
    {
      "role": "user",
      "content": [
        {
          "type": "input_audio",
          "input_audio": {
            "data": "$(base64 -w 0 samples/filename.wav)",
            "format": "wav"
          }
        },
        {
          "type": "text",
          "text": "Transcribe the audio into a Markdown list. Output ONLY the list."
        }
      ]
    }
  ],
  "temperature": 0.0
}
EOF
```

# llama-server CPU mode

```bash
./llama-server   -m models/Llama-3.2-1B-Instruct-Q4_K_M.gguf --ctx-size 8192 --n-gpu-layers 0 --threads 4
```

# llama-server GPU mode

```bash
./llama-server   -m models/Llama-3.2-1B-Instruct-Q4_K_M.gguf --ctx-size 8192 --n-gpu-layers 99
```
