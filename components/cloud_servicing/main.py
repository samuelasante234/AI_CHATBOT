from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()

@app.get("/")
def server_init():
    return {"Status": "Active"}
class PayloadStructure(BaseModel):
    message : str

@app.post("/code")
def function_to_validate(var: PayloadStructure):
    return {"status_message": var.message}