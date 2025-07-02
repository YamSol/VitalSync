#!/usr/bin/env python3
# VitalSync Medical Data API - Adapted for ESP32 Gateway

from fastapi import FastAPI, Depends, HTTPException, Header
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import List, Optional
from datetime import datetime
import os
import logging

# ------------------------
# Configuração de logging
# ------------------------
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(title="VitalSync Medical Data API", version="1.0.0")

# ------------------------
# CORS - Permitir frontend se conectar
# ------------------------
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Em produção, especifique os domínios permitidos
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ------------------------
# Diretório para salvar dados (se necessário)
# ------------------------
DATA_DIR = "data"
os.makedirs(DATA_DIR, exist_ok=True)

# ------------------------
# "Banco de dados" em memória
# ------------------------
users_db = [
    {"id": 1, "email": "igor@inatel.com", "password": "123456", "name": "Igor"},
    {"id": 2, "email": "yam@inatel.com", "password": "senha123", "name": "Yam"},
]

patients_db = [
    {"id": "123", "name": "Igor", "date_of_birth": "2003-05-30"},
    {"id": "456", "name": "Yam", "date_of_birth": "1985-07-15"},
]

latest_device_data = {}  # Armazena os dados mais recentes por device_id

# ------------------------
# Models
# ------------------------

class LoginRequest(BaseModel):
    email: str
    password: str

class UserProfile(BaseModel):
    id: int
    name: str
    email: str

class LoginResponse(BaseModel):
    message: str
    user_id: int
    name: str

class BloodPressure(BaseModel):
    systolic: int
    diastolic: int

class DeviceMedicalData(BaseModel):
    device_id: str
    heart_rate: int
    oxygen_level: int
    pressure: BloodPressure
    temperature: float
    timestamp: datetime

class Patient(BaseModel):
    id: str
    name: str
    date_of_birth: str

class PatientUpdate(BaseModel):
    name: Optional[str]
    date_of_birth: Optional[str]

# ------------------------
# Função de autenticação
# ------------------------

def authenticate_user(email: str, password: str) -> Optional[UserProfile]:
    for user in users_db:
        if user["email"] == email and user["password"] == password:
            return UserProfile(id=user["id"], name=user["name"], email=user["email"])
    return None

# ------------------------
# Authentication Endpoints
# ------------------------

@app.post("/auth/login", response_model=LoginResponse)
def login_user(credentials: LoginRequest):
    user = authenticate_user(credentials.email, credentials.password)
    if not user:
        raise HTTPException(status_code=401, detail="Invalid email or password")
    return {
        "message": "Login successful",
        "user_id": user.id,
        "name": user.name
    }

@app.post("/auth/logout")
def logout_user():
    return {"message": "Logged out"}

@app.get("/auth/profile", response_model=UserProfile)
def get_profile():
    return UserProfile(id=1, name="John Doe", email="john@example.com")

# ------------------------
# Gateway Data Submission + Armazenamento
# ------------------------

@app.post("/gateway/device-data")
def receive_device_data(payload: DeviceMedicalData, x_api_key: str = Header(...)):
    if x_api_key != "expected-api-key":
        raise HTTPException(status_code=401, detail="Invalid API key")

    # Armazena os dados mais recentes por device_id
    latest_device_data[payload.device_id] = payload

    logger.info(f"Dados recebidos do dispositivo {payload.device_id}")
    
    return {
        "status": "received",
        "device_id": payload.device_id,
        "timestamp": payload.timestamp,
        "data_summary": {
            "heart_rate": payload.heart_rate,
            "oxygen_level": payload.oxygen_level,
            "temperature": payload.temperature,
            "pressure": {
                "systolic": payload.pressure.systolic,
                "diastolic": payload.pressure.diastolic
            }
        }
    }

# ------------------------
# Endpoint para dados mais recentes do dispositivo
# ------------------------

@app.get("/device/{device_id}/latest", response_model=DeviceMedicalData)
def get_latest_device_data(device_id: str):
    data = latest_device_data.get(device_id)
    if not data:
        raise HTTPException(status_code=404, detail="No data found for this device")
    return data

# ------------------------
# CRUD de Pacientes
# ------------------------

@app.get("/patients", response_model=List[Patient])
def list_patients():
    return patients_db

@app.get("/patients/{id}", response_model=Patient)
def get_patient(id: str):
    for p in patients_db:
        if p["id"] == id:
            return p
    raise HTTPException(status_code=404, detail="Patient not found")

@app.post("/patients", response_model=Patient)
def create_patient(patient: Patient):
    if any(p["id"] == patient.id for p in patients_db):
        raise HTTPException(status_code=400, detail="Patient ID already exists")
    patients_db.append(patient.dict())
    return patient

@app.put("/patients/{id}", response_model=Patient)
def update_patient(id: str, updates: PatientUpdate):
    for p in patients_db:
        if p["id"] == id:
            if updates.name:
                p["name"] = updates.name
            if updates.date_of_birth:
                p["date_of_birth"] = updates.date_of_birth
            return p
    raise HTTPException(status_code=404, detail="Patient not found")

@app.delete("/patients/{id}")
def delete_patient(id: str):
    for i, p in enumerate(patients_db):
        if p["id"] == id:
            del patients_db[i]
            return {"status": "deleted", "id": id}
    raise HTTPException(status_code=404, detail="Patient not found")

# ------------------------
# Dados médicos (histórico fictício)
# ------------------------

@app.get("/patients/{id}/medical-data")
def get_medical_history(id: str):
    return {"patient_id": id, "history": []}

@app.get("/patients/{id}/medical-data/latest")
def get_latest_data(id: str):
    return {"patient_id": id, "latest": {"heart_rate": 75, "timestamp": datetime.utcnow()}}
