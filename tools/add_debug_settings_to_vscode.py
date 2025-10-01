import json
import os

# Пути к файлам
settings_path = os.path.join('.vscode', 'settings.json')
additional_settings_path = './tools/vscode_debug_settings.json'

# Загрузка дополнительного JSON объекта из файла vscode_debug_settings.json
if os.path.exists(additional_settings_path):
    with open(additional_settings_path, 'r', encoding='utf-8') as f:
        try:
            additional_settings = json.load(f)
        except json.JSONDecodeError:
            print(f"Ошибка чтения JSON из {additional_settings_path}, файл пропущен.")
            additional_settings = {}
else:
	additional_settings = {}
	print(f"Файл {additional_settings_path} не найден, дополнительный объект пуст.")

print(settings_path)

# Загружаем существующие настройки из settings.json
if os.path.exists(settings_path):
    with open(settings_path, 'r', encoding='utf-8') as f:
        try:
            settings = json.load(f)
        except json.JSONDecodeError:
            settings = {}
else:
    settings = {}

# Обновляем настройки дополнительным объектом
settings.update(additional_settings)

# Записываем обратно в файл settings.json
os.makedirs(os.path.dirname(settings_path), exist_ok=True)
with open(settings_path, 'w', encoding='utf-8') as f:
    json.dump(settings, f, indent=4, ensure_ascii=False)

print(f"Updated {settings_path} successfully.")
