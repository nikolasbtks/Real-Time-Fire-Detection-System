from flask import Flask, request, jsonify
from datetime import datetime
import numpy as np
import joblib

app = Flask(__name__)

model = joblib.load("model.pkl")

@app.route('/')
def home():
	return ""
	
@app.route('/process', methods=['POST'])
def process():
	data = request.get_json()
	
	if not data:
		return jsonify({"error": "Error processing your JSON"}), 400
		
	manufacture_code = data.get("manufacture_code","device")
	
	if not manufacture_code:
		return jsonify({"error": "Error processing Manufacturing code"}), 400
		
	request_sequence = data.get("sequence",997)
	
	if not isinstance(request_sequence, list) or not len(request_sequence) == 10 or None in request_sequence:
		return jsonify({"error": "Error processing the sequence"}), 400
		
	converted_sequence = np.array(request_sequence)
	
	sequence = np.array(converted_sequence)
	std_feature = np.std(sequence)
	minimum_feature = np.min(sequence)
	maximum_feature = np.max(sequence)
	mean_feature = np.mean(sequence)
	range_feature = maximum_feature - minimum_feature
	diffs = np.diff(sequence)
	mean_d = np.mean(diffs) if len(diffs) > 0 else 0
	
	features_sequence = np.array([mean_feature,std_feature,minimum_feature,maximum_feature,range_feature,mean_d]).reshape(1,-1)
	
	prediction = model.predict(features_sequence)
	predicted_result = prediction[0]
	
	if predicted_result == 0:
		result_class = "No Fire"
	elif predicted_result == 1:
		result_class = "Suspicious Activity"
	else:
		result_class = "Fire"
	
	current_timestamp = datetime.now()
	day = current_timestamp.day
	month = current_timestamp.month 
	year = current_timestamp.year 
	hour = current_timestamp.hour 
	minute = current_timestamp.minute 
	second = current_timestamp.second 
	millisecond = current_timestamp.microsecond // 1000 
	
	timestamp = f"Date: {day:02d}-{month:02d}-{year}, Time: {hour:02d}:{minute:02d}:{second:02d}.{millisecond:03d}"
	
	response = {
		"manufacture_code": manufacture_code,
		"result": result_class,
		"timestamp": timestamp
	}
	
	return jsonify(response), 200
	
if __name__ == '__main__':
	app.run(host='0.0.0.0', port=5000)
	
	
	
	
	
	
	
