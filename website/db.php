<?php
$host = "localhost";
$user = "root";
$pass = ""; // در لاراگون پسورد پیش‌فرض خالی است
$dbname = "greenhouse_db";

try {
    $pdo = new PDO("mysql:host=$host;dbname=$dbname;charset=utf8", $user, $pass);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    die("Connection failed: " . $e->getMessage());
}
?>