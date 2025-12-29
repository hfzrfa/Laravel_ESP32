<?php

use App\Http\Controllers\Api\DeviceStateController;
use App\Http\Controllers\Api\SensorReadingController;
use Illuminate\Support\Facades\Route;

Route::middleware('device.auth')->group(function (): void {
    Route::post('/sensor-readings', [SensorReadingController::class, 'store']);
    Route::get('/device/state', [DeviceStateController::class, 'show']);
    Route::post('/device/alarm/ack', [DeviceStateController::class, 'acknowledgeAlarm']);
});

Route::get('/sensor-readings/latest', [SensorReadingController::class, 'latest']);
