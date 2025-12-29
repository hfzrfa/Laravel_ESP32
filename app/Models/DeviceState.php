<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class DeviceState extends Model
{
    use HasFactory;

    protected $fillable = [
        'lcd_mode',
        'lcd_custom_text',
        'oled_mode',
        'oled_custom_text',
        'led_mode',
        'buzzer_enabled',
        'alarm_time',
        'alarm_enabled',
        'alarm_last_triggered_at',
        'last_ping_at',
        'last_reported_ip',
    ];

    protected $casts = [
        'buzzer_enabled' => 'boolean',
        'alarm_enabled' => 'boolean',
        'alarm_time' => 'string',
        'alarm_last_triggered_at' => 'datetime',
        'last_ping_at' => 'datetime',
    ];

    public static function singleton(): self
    {
        return static::first() ?? static::create([
            'lcd_mode' => 'clock',
            'oled_mode' => 'clock',
            'led_mode' => 'auto',
        ]);
    }
}
