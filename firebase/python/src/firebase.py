from datetime import time
from os import path
from inspect import getsourcefile
import requests

import firebase_admin
from firebase_admin import credentials, db


source = getsourcefile(lambda: 0) or ''
cur_dir = path.dirname(source)

cert_file = path.join('config', 'firebase-admin-cert.json')
f = path.join(cur_dir, cert_file)
project_id = ''
with open(f) as json_file:
    project_id = firebase_admin.json.load(json_file).get('project_id')


class Firebase:
    def __init__(self) -> None:
        self.cred = cred = credentials.Certificate(f)
        token = self.cred.get_access_token()
        configData = requests.get(
            f'https://firebase.googleapis.com/v1beta1/projects/{project_id}/adminSdkConfig', params={
                'access_token': f'{token.access_token}'
            })
        dbURL = configData.json().get('databaseURL')
        self.app = firebase_admin.initialize_app(cred, {
            'databaseURL': dbURL
        })

    def send(self, data: dict | str, data_path='/'):
        ref = db.reference(path=data_path)
        ref.set(data)


firebase = Firebase()
firebase.send({'timestamp': 0}, data_path='/temp')
