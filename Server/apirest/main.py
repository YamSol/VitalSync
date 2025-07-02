from fastapi import FastAPI, Query

app = FastAPI()

items = []

@app.post("/items")
def create_item(item: str = Query(...)):
    items.append(item)
    return items
