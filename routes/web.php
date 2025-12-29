<?php

use App\Http\Controllers\DashboardController;
use Illuminate\Support\Facades\Route;

Route::get('/', [DashboardController::class, 'index'])->name('dashboard');
Route::post('/controls/displays', [DashboardController::class, 'updateDisplays'])->name('controls.displays');
Route::post('/controls/alarm', [DashboardController::class, 'updateAlarm'])->name('controls.alarm');
Route::post('/controls/outputs', [DashboardController::class, 'updateOutputs'])->name('controls.outputs');
