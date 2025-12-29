<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\DeviceState;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;
use Illuminate\Validation\Rule;

class DeviceStateController extends Controller
{
    public function show(): JsonResponse
    {
        $state = DeviceState::singleton();

        return response()->json([
            'data' => [
                'lcd_mode' => $state->lcd_mode,
                'lcd_custom_text' => $state->lcd_custom_text,
                'oled_mode' => $state->oled_mode,
                'oled_custom_text' => $state->oled_custom_text,
                'led_mode' => $state->led_mode,
                'buzzer_enabled' => $state->buzzer_enabled,
                'alarm' => [
                    'enabled' => $state->alarm_enabled,
                    'time' => $state->alarm_time,
                    'last_triggered_at' => $state->alarm_last_triggered_at,
                ],
                'server_time' => now()->toIso8601String(),
            ],
        ]);
    }

    public function acknowledgeAlarm(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'triggered_at' => ['nullable', 'date'],
        ]);

        $state = DeviceState::singleton();
        $state->update([
            'alarm_enabled' => false,
            'alarm_last_triggered_at' => $validated['triggered_at'] ?? now(),
        ]);

        return response()->json([
            'message' => 'Alarm acknowledged.',
            'state' => $state,
        ]);
    }
}
