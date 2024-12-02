from flask import Flask, request, render_template_string, send_from_directory, redirect, jsonify
import os
import time

app = Flask(__name__)

IMAGES_DIR = '/images'
NOTES_FILE = '/data/notes.txt'

os.makedirs(IMAGES_DIR, exist_ok=True)
os.makedirs(os.path.dirname(NOTES_FILE), exist_ok=True)

if not os.path.exists(NOTES_FILE):
   with open(NOTES_FILE, 'w') as f:
       pass

TEMPLATE = '''
<!DOCTYPE html>
<html>
<head>
   <title>Docker Data Demo</title>
   <style>
       body { font-family: Arial; margin: 40px; }
       .container { display: flex; gap: 40px; }
       .section { flex: 1; }
       img { max-width: 300px; }
       .note { margin: 10px 0; padding: 10px; background: #f0f0f0; display: flex; justify-content: space-between; }
       .note button { margin-left: 10px; }
       .edit-form { display: none; }
       .edit-form.active { display: block; }
   </style>
</head>
<body>
   <h1>Docker Data Demo</h1>
   <button onclick="location.reload()">Обновить страницу</button>
   <div class="container">
       <div class="section">
           <h2>Images Section</h2>
           {% if images %}
               <img src="/images/{{ current_image }}" alt="Current Image">
               <p>Available images: {{ ', '.join(images) }}</p>
           {% else %}
               <p>No images available</p>
           {% endif %}
       </div>
       
       <div class="section">
           <h2>Notes Section</h2>
           <form method="post" action="/add_note">
               <input type="text" name="note" placeholder="Enter your note">
               <input type="submit" value="Add Note">
           </form>
           <div class="notes">
               {% for note in notes %}
                   <div class="note" id="note-{{ loop.index0 }}">
                       <span class="note-text">{{ note }}</span>
                       <div>
                           <button onclick="showEdit({{ loop.index0 }})">Изменить</button>
                           <button onclick="deleteNote({{ loop.index0 }})">Удалить</button>
                       </div>
                   </div>
                   <div class="edit-form" id="edit-{{ loop.index0 }}">
                       <input type="text" value="{{ note }}" id="edit-input-{{ loop.index0 }}">
                       <button onclick="saveEdit({{ loop.index0 }})">Сохранить</button>
                   </div>
               {% endfor %}
           </div>
       </div>
   </div>
   <script>
       function showEdit(index) {
           document.getElementById(`edit-${index}`).classList.add('active');
       }
       
       function saveEdit(index) {
           const newText = document.getElementById(`edit-input-${index}`).value;
           fetch('/edit_note', {
               method: 'POST',
               headers: {
                   'Content-Type': 'application/json',
               },
               body: JSON.stringify({
                   index: index,
                   new_text: newText
               })
           }).then(() => location.reload());
       }
       
       function deleteNote(index) {
           fetch('/delete_note', {
               method: 'POST',
               headers: {
                   'Content-Type': 'application/json',
               },
               body: JSON.stringify({index: index})
           }).then(() => location.reload());
       }
   </script>
</body>
</html>
'''

@app.route('/')
def index():
   try:
       images = os.listdir(IMAGES_DIR)
   except Exception as e:
       app.logger.error(f"Error listing images: {e}")
       images = []
   
   current_image = images[int(time.time()) % len(images)] if images else None
   
   notes = []
   if os.path.exists(NOTES_FILE):
       with open(NOTES_FILE, 'r') as f:
           notes = [line.strip() for line in f.readlines()]
   
   return render_template_string(TEMPLATE, images=images, current_image=current_image, notes=notes)

@app.route('/edit_note', methods=['POST'])
def edit_note():
   data = request.get_json()
   index = data['index']
   new_text = data['new_text']
   
   notes = []
   with open(NOTES_FILE, 'r') as f:
       notes = [line.strip() for line in f.readlines()]
   
   notes[index] = new_text
   
   with open(NOTES_FILE, 'w') as f:
       f.write('\n'.join(notes) + '\n')
   
   return jsonify({'success': True})

@app.route('/delete_note', methods=['POST'])
def delete_note():
   data = request.get_json()
   index = data['index']
   
   notes = []
   with open(NOTES_FILE, 'r') as f:
       notes = [line.strip() for line in f.readlines()]
   
   notes.pop(index)
   
   with open(NOTES_FILE, 'w') as f:
       f.write('\n'.join(notes))
   
   return jsonify({'success': True})

@app.route('/images/<path:filename>')
def serve_image(filename):
   return send_from_directory(IMAGES_DIR, filename)

@app.route('/add_note', methods=['POST'])
def add_note():
   note = request.form.get('note')
   if note:
       with open(NOTES_FILE, 'a') as f:
           f.write(note + '\n')
   return redirect('/')

if __name__ == '__main__':
   app.run(host='0.0.0.0', port=5000, debug=True)