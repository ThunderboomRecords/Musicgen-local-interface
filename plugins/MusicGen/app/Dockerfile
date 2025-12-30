# Use a lightweight Python image
FROM python:3.9-slim

# Set the working directory
WORKDIR /app

# Copy the application files
COPY . /app

# Install dependencies
RUN pip install --no-cache-dir flask

# Expose the Flask port
EXPOSE 5000

# Run the Flask app
CMD ["python", "app.py"]
