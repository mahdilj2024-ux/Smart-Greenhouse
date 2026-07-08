<?php
header('Content-Type: application/json');
require_once 'db.php'; // Connects to greenhouse_db using your db.php parameters

// Read raw POST data from Raspberry Pi
$json = file_get_contents('php://input');
$data = json_decode($json, true);

if (!$data) {
    echo json_encode(["status" => "error", "message" => "No JSON data received"]);
    exit;
}

// Extract topic and variables
$topic = $data['topic'] ?? '';
$line_id = 'main';

// Determine line_id from MQTT topic structure
if (strpos($topic, 'greenhouse/1/') !== false) {
    $line_id = '1';
} elseif (strpos($topic, 'greenhouse/2/') !== false) {
    $line_id = '2';
} elseif (strpos($topic, 'greenhouse/3/') !== false) {
    $line_id = '3';
}

// Map parameters safely (handles "inf" or missing fields by turning them into NULL)
$temp = (isset($data['temp']) && $data['temp'] !== 'inf') ? floatval($data['temp']) : null;
$hemu = isset($data['hemu']) ? floatval($data['hemu']) : null;
$press = isset($data['press']) ? floatval($data['press']) : null;
$soil = isset($data['soilhemu']) ? floatval($data['soilhemu']) : null;

// Normalize pump/lamp status to matching string database format
$pump = null;
if (isset($data['fan'])) {
    $pump = $data['fan'];
} elseif (isset($data['lamp'])) {
    $pump = $data['lamp'];
} elseif (isset($data['pump'])) {
    $pump = $data['pump'];
}

try {
    // Insert structured packet into telemetry_data table
    $query = "INSERT INTO telemetry_data (line_id, temperature, humidity, pressure, soil_humidity, pump_status) 
              VALUES (:line_id, :temp, :hemu, :press, :soil, :pump)";
              
    $stmt = $pdo->prepare($query);
    $stmt->execute([
        ':line_id' => $line_id,
        ':temp' => $temp,
        ':hemu' => $hemu,
        ':press' => $press,
        ':soil' => $soil,
        ':pump' => $pump
    ]);

    echo json_encode(["status" => "success", "message" => "Data saved correctly"]);
} catch (PDOException $e) {
    echo json_encode(["status" => "error", "message" => $e->getMessage()]);
}
?>