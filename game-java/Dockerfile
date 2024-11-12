FROM openjdk:11

LABEL version="1.0.1"
LABEL description="Java Guessing Game in Docker"

WORKDIR /app

COPY game.java .

RUN javac game.java

CMD ["java", "game"]