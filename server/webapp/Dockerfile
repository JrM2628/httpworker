FROM python:3.8-slim-buster
WORKDIR /app
COPY . .
RUN pip3 install -r requirements.txt
WORKDIR c2
CMD [ "python3", "-m" , "flask", "run", "--host=0.0.0.0" ]