import socket
import os
import uuid
import torchaudio
from audiocraft.models import MusicGen
from audiocraft.data.audio import audio_write
import sys
sys.setrecursionlimit(5000)

AUDIO_FOLDER = f'.{os.sep}'

# Load models
model_prompt = MusicGen.get_pretrained("facebook/musicgen-stereo-small", device='cpu')
# model_melody = MusicGen.get_pretrained("facebook/musicgen-stereo-melody", device='cpu')
model_melody = None
print('Done loading models')

# Replace this function with your actual audio generation logic
def generate_audio(prompt, temperature, topk, topp, cfg, samples, duration, dropped=False):
    audio_file_paths = [os.path.join(AUDIO_FOLDER, f"{uuid.uuid4()}") for _ in range(samples)]
    
    if dropped:
        melody, sr = torchaudio.load(dropped)
        model_melody.set_generation_params(
            duration=duration,
            temperature=temperature,
            top_k=topk,
            top_p=topp,
            cfg_coef=cfg
        )
        for audio_file_path in audio_file_paths:
            wav = model_melody.generate_with_chroma([prompt], melody[None].expand(1, -1, -1), sr)
            audio_write(audio_file_path, wav[0].cpu(), model_melody.sample_rate, strategy="loudness")
    else:
        model_prompt.set_generation_params(
            duration=duration,
            temperature=temperature,
            top_k=topk,
            top_p=topp,
            cfg_coef=cfg
        )
        for audio_file_path in audio_file_paths:
            wav = model_prompt.generate([prompt])
            audio_write(audio_file_path, wav[0].cpu(), model_prompt.sample_rate, strategy="loudness")

    return [f"{audio_file_path}.wav" for audio_file_path in audio_file_paths]

def start_socket_server(host='127.0.0.1', port=65432):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(5)
    print(f"Server listening on {host}:{port}...")

    while True:
        conn, addr = server_socket.accept()
        print(f"Connected by {addr}")

        # Receive the request from the client
        data = conn.recv(4096).decode('utf-8')
        if not data:
            break
        print(f"Received request: {data}")

        # Expecting the data to be in the format: 'prompt|temperature|topk|topp|cfg|samples|duration'
        params = data.split('|')
        if len(params) == 7:
            prompt, temperature, topk, topp, cfg, samples, duration = params
            temperature = float(temperature)
            topk = int(topk)
            topp = float(topp)
            cfg = int(cfg)
            samples = int(samples)
            duration = int(duration)

            # Generate audio
            audio_file_paths = generate_audio(prompt, temperature, topk, topp, cfg, samples, duration)
            audio_filenames = [os.path.basename(audio_file_path) for audio_file_path in audio_file_paths]
            response = '|'.join(audio_filenames)

            # Send response back to client
            conn.sendall(response.encode('utf-8'))
        else:
            conn.sendall(b'Invalid request format')
        
        # Close the connection
        conn.close()

if __name__ == "__main__":
    start_socket_server()
