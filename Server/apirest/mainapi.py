#!/usr/bin/env python3
# VitalSync Medical Data API - Adapted for ESP32 Gateway
from fastapi import FastAPI, Depends, HTTPException, Header
from fastapi.middleware.cors import CORSMiddleware
from fastapi.security import OAuth2PasswordRequestForm
from pydantic import BaseModel, Field
from typing import List, Optional
from datetime import datetime
import json
import os
import logging

# Configuração de logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(title="VitalSync Medical Data API", version="1.0.0")

# CORS para aceitar requisições do ESP32
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Em produção, especificar origins
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Diretório para salvar dados
DATA_DIR = "data"
os.makedirs(DATA_DIR, exist_ok=True)

# ------------------------
# Authentication Models
# ------------------------
class LoginRequest(BaseModel):
    email: str
    password: str

class UserProfile(BaseModel):
    id: int
    name: str
    email: str

# ------------------------
# Medical Data Models
# ------------------------
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

# ------------------------
# Patient Models
# ------------------------
class Patient(BaseModel):
    id: str
    name: str
    date_of_birth: str

class PatientUpdate(BaseModel):
    name: Optional[str]
    date_of_birth: Optional[str]

# ------------------------
# Authentication Endpoints
# ------------------------
@app.post("/auth/login")
def login_user(credentials: LoginRequest):
    # Placeholder logic
    return {"access_token": "jwt-token", "token_type": "bearer"}

@app.post("/auth/logout")
def logout_user():
    # Placeholder for token invalidation
    return {"message": "Logged out"}

@app.get("/auth/profile", response_model=UserProfile)
def get_profile():
    # Dummy user profile
    return UserProfile(id=1, name="John Doe", email="john@example.com")

# ------------------------
# Gateway Data Submission
# ------------------------
@app.post("/gateway/device-data")
def receive_device_data(payload: DeviceMedicalData, x_api_key: str = Header(...)):
    if x_api_key != "expected-api-key":
        raise HTTPException(status_code=401, detail="Invalid API key")
    
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
# Patient CRUD Endpoints
# ------------------------
@app.get("/patients", response_model=List[Patient])
def list_patients():
    return [Patient(id="123", name="Alice", date_of_birth="1990-01-01")]

@app.get("/patients/{id}", response_model=Patient)
def get_patient(id: str):
    return Patient(id=id, name="Alice", date_of_birth="1990-01-01")

@app.post("/patients", response_model=Patient)
def create_patient(patient: Patient):
    return patient

@app.put("/patients/{id}", response_model=Patient)
def update_patient(id: str, updates: PatientUpdate):
    return Patient(
        id=id,
        name=updates.name or "Alice",
        date_of_birth=updates.date_of_birth or "1990-01-01"
    )

@app.delete("/patients/{id}")
def delete_patient(id: str):
    return {"status": "deleted", "id": id}

# ------------------------
# Medical Data Access Endpoints
# ------------------------
@app.get("/patients/{id}/medical-data")
def get_medical_history(id: str):
    return {"patient_id": id, "history": []}

@app.get("/patients/{id}/medical-data/latest")
def get_latest_data(id: str):
    return {"patient_id": id, "latest": {"heart_rate": 75, "timestamp": datetime.utcnow()}}
