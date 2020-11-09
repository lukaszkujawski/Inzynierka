<?php
define('DB_SERVER', 'localhost');
define('DB_USERNAME', 'mysql_user');
define('DB_PASSWORD', 'serwerek@123');
define('DB_NAME', 'CO');

$sql_conn = mysqli_connect(DB_SERVER, DB_USERNAME, DB_PASSWORD, DB_NAME);

if (!$sql_conn) {
    die("Connection failed: " .mysqli_connect_error());
}

$zapytanie = "SELECT * FROM Config ORDER BY ID DESC LIMIT 1";

$idzapytania = mysqli_query($sql_conn, $zapytanie);
$row = mysqli_fetch_assoc($idzapytania);

$obj = new stdClass();
$obj->kp = $row["KP"];
$obj->ki = $row["KI"];
$obj->kd = $row["KD"];
$obj->temperature_set = $row["TEMPERATURE_SET"];
$obj->pid_set = $row["PID_SET"];
$obj->manual_mode = $row["MANUAL_MODE"];

echo json_encode($obj);

mysqli_close($sql_conn);
