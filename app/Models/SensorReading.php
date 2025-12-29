<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class SensorReading extends Model
{
    use HasFactory;

    protected $fillable = [
        'temperature_c',
        'humidity',
        'battery_level',
        'signal_strength',
        'recorded_at',
    ];

    protected $casts = [
        'temperature_c' => 'float',
        'humidity' => 'float',
        'battery_level' => 'float',
        'recorded_at' => 'datetime',
    ];
}
