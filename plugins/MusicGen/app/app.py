from flask import Flask, request, jsonify, send_from_directory, render_template, abort
from transformers import AutoProcessor, MusicgenForConditionalGeneration, MusicgenMelodyForConditionalGeneration
import os, torchaudio, uuid, glob, time, shutil
from pathlib import Path
import torch
import soundfile as sf
from transformers import pipeline
import platform

current_os = platform.system()

if current_os == "Windows" or current_os == "Linux":
    device = 'cuda:0' if torch.cuda.is_available() else 'cpu'
    # TODO: add calc for GPU mem
elif current_os == "Darwin":
    device = 'cpu' # Would love to use mps but there is a limit in the number of output channels on mps.

AUDIO_FOLDER = os.path.join(Path.home(), 'audio')
os.makedirs(AUDIO_FOLDER, exist_ok=True)

processor_small = AutoProcessor.from_pretrained("facebook/musicgen-stereo-small")
model_small = MusicgenForConditionalGeneration.from_pretrained("facebook/musicgen-stereo-small").to(device)
processor_melody = AutoProcessor.from_pretrained("facebook/musicgen-stereo-melody")
model_melody = MusicgenMelodyForConditionalGeneration.from_pretrained("facebook/musicgen-stereo-melody").to(device)

# Initialize the Flask application
app = Flask(__name__, static_url_path='/')

# Replace this function with your actual audio generation logic
def generate_audio(prompt, temperature, topk, topp, cfg, samples, duration, dropped=False, userid=None):
    audio_file_paths = [os.path.join(AUDIO_FOLDER, f"{uuid.uuid4()}") for _ in range(samples)]
    txt_file = '\n'.join(audio_file_paths)
    os.system(f'echo "{txt_file}" > {os.path.join(AUDIO_FOLDER, "recent_audio.txt")}')
    if dropped:
        melody, sr = torchaudio.load(dropped)
        inputs = processor_small(
            audio=melody[0],
            sampling_rate=sr,
            text=[prompt],
            padding=True,
            return_tensors="pt",
        ).to(device)
    else:
        inputs = processor_small(
            text=[prompt],
            padding=True,
            return_tensors="pt",
        ).to(device)

    for audio_file_path in audio_file_paths:
        wav = model_small.generate(
            **inputs, 
            max_new_tokens=int(float(duration)*50),
            temperature=temperature,
            top_k=topk,
            top_p=topp,
            guidance_scale=cfg,
        )
        sf.write(audio_file_path, wav[0].T.Tcpu(), 32000)

    return [f"{audio_file_path}.wav" for audio_file_path in audio_file_paths ]

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        prompt = str(request.form.get('prompt')).lower()
        userid = str(request.form.get('userid'))
        if userid == 'undefined' or userid not in os.listdir(AUDIO_FOLDER):
            userid = str(uuid.uuid4())
            os.makedirs(f"{AUDIO_FOLDER}/{userid}", exist_ok=True)
        else:
            for file_ in glob.glob(f"{AUDIO_FOLDER}/{userid}/*"):
                os.unlink(file_)
        # Handle the uploaded audio file
        audio_file = request.files.get('audioInput')
        dropped = False
        if audio_file and audio_file.filename != '':
            dropped = os.path.join(AUDIO_FOLDER, userid, f"{uuid.uuid4()}.wav")
            audio_file.save(dropped)
        if prompt:
            try:
                temperature = float(request.form.get('Temperature'))
                topk = int(request.form.get('Top K'))
                topp = float(request.form.get('Top P'))
                cfg = int(request.form.get('Classifier Free Guidance'))
                samples = int(request.form.get('Samples'))
                duration = float(request.form.get('Duration'))
                audio_file_paths = generate_audio(prompt, temperature, topk, topp, cfg, samples, duration, dropped=dropped, userid=userid)
                audio_filenames = [os.path.basename(audio_file_path) for audio_file_path in audio_file_paths]
                download_links = [f'/download/{userid}/{audio_filename}' for audio_filename in audio_filenames]
                return jsonify({'success': True, 'download_links': download_links, 'userid': userid})
            except Exception as e:
                print(e)
                return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/download_links', methods=['POST'])
def get_download_links():
    userid = str(request.json.get('userid'))
    if userid == 'undefined' or userid not in os.listdir(AUDIO_FOLDER):
        userid = str(uuid.uuid4())
        os.makedirs(f"{AUDIO_FOLDER}/{userid}", exist_ok=True)
    try:
        if os.path.exists(os.path.join(AUDIO_FOLDER, userid, 'recent_audio.txt')):
            with open(os.path.join(AUDIO_FOLDER, userid, 'recent_audio.txt'), "r") as file_:
                audio_paths = [path for path in file_.read().split('\n') if os.path.exists(path)]
                audio_filenames = [os.path.basename(audio_file_path) for audio_file_path in audio_paths]
        else:
            audio_filenames = []
        download_links = [f'/download/{userid}/{audio_filename}.wav' for audio_filename in audio_filenames]
        return jsonify({'success': True, 'download_links': download_links, 'userid': userid})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/download/<userid>/<filename>')
def download(userid, filename):
    return send_from_directory(os.path.join(AUDIO_FOLDER, userid), filename, as_attachment=True)
    
if __name__ == '__main__':
    app.run(debug=True, port=55000)