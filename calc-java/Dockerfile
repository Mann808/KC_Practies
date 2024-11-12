FROM openjdk:11

LABEL version="1.0.1"
LABEL description="Java Calculator in Docker"

WORKDIR /app

COPY calc.java .

RUN javac calc.java

CMD ["java", "calc"]