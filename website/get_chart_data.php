<?php
header('Content-Type: application/json');
require_once 'db.php';

$line = $_GET['line'] ?? 'main';
$metric = $_GET['metric'] ?? 'temp';
$range = $_GET['range'] ?? 'today';
$customDate = $_GET['custom_date'] ?? '';

// تعیین بازه زمانی در SQL
$timeCondition = "WHERE line_id = :line ";
if ($range == 'today') {
    $timeCondition .= "AND DATE(created_at) = CURDATE()";
} elseif ($range == 'yesterday') {
    $timeCondition .= "AND DATE(created_at) = SUBDATE(CURDATE(), 1)";
} elseif ($range == 'week') {
    $timeCondition .= "AND created_at >= NOW() - INTERVAL 7 DAY";
} elseif ($range == 'month') {
    $timeCondition .= "AND created_at >= NOW() - INTERVAL 30 DAY";
} elseif ($range == 'custom' && !empty($customDate)) {
    $timeCondition .= "AND DATE(created_at) = :custom_date";
}

// انتخاب ستون مورد نظر بر اساس متریک انتخابی
$column = 'temperature';
if ($metric == 'hemu') $column = 'humidity';
if ($metric == 'press') $column = 'pressure';
if ($metric == 'soil') $column = 'soil_humidity';
if ($metric == 'pump') $column = 'pump_status';

// دریافت داده‌ها (برای اینکه نمودار شلوغ نشود محدود به ۵۰ داده آخر می‌کنیم)
$query = "SELECT DATE_FORMAT(created_at, '%H:%i') as time_label, $column as value 
          FROM telemetry_data 
          $timeCondition 
          ORDER BY id DESC LIMIT 50";

try {
    $stmt = $pdo->prepare($query);
    $stmt->bindValue(':line', $line);
    if ($range == 'custom' && !empty($customDate)) {
        $stmt->bindValue(':custom_date', $customDate);
    }
    $stmt->execute();
    $result = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // مرتب‌سازی صعودی داده‌ها برای رسم درست روی نمودار زمان
    $result = array_reverse($result);
    
    echo json_encode($result);
} catch (PDOException $e) {
    echo json_encode(["error" => $e->getMessage()]);
}
?>