FROM python:3.9-slim

WORKDIR /app

RUN mkdir -p /images /data && \
    chmod 777 /images && \
    chmod 777 /data

COPY requirements.txt .
RUN pip install -r requirements.txt

COPY . .

CMD ["python", "app.py"]