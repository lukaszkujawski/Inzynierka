<?php
define('DB_SERVER', 'localhost');
define('DB_USERNAME', 'mysql_user');
define('DB_PASSWORD', 'serwerek@123');
define('DB_NAME', 'CO');

date_default_timezone_set('Europe/Warsaw');

$date = date("Y-m-d H:i:s");

$sql_conn = mysqli_connect(DB_SERVER, DB_USERNAME, DB_PASSWORD, DB_NAME);

if (!$sql_conn) {
    die("Connection failed: " .mysqli_connect_error());
}

$temperature_set = $_POST["temperature_set"];
$PID_set = $_POST["PID_set"];
$manual_mode = $_POST["manual_mode"];
$kp = $_POST["kp"];
$ki = $_POST["ki"];
$kd = $_POST["kd"];

$inserting = "INSERT INTO Config (SETTING_DATE, KP, KI, KD, TEMPERATURE_SET, PID_SET, MANUAL_MODE) 
VALUES ('{$date}', '{$kp}', '{$ki}', '{$kd}', '{$temperature_set}', '{$PID_set}', '{$manual_mode}')";

if(mysqli_query($sql_conn, $inserting) > 0) {
    echo "OK written config data";
} else {
    die("Error writing config data " . mysqli_error($sql_conn));
}

mysqli_close($sql_conn);
