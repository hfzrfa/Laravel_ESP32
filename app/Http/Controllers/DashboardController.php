<?php

namespace App\Http\Controllers;

use App\Models\DeviceState;
use App\Models\SensorReading;
use Illuminate\Http\RedirectResponse;
use Illuminate\Http\Request;
use Illuminate\Http\Response;
use Illuminate\Support\Arr;
use Illuminate\Validation\Rule;

class DashboardController extends Controller
{
    public function index(): Response
    {
        $state = DeviceState::singleton();
        $latestReading = SensorReading::orderByDesc('recorded_at')
            ->orderByDesc('created_at')
            ->first();

        return response()->view('welcome', [
            'state' => $state,
            'latestReading' => $latestReading,
        ]);
    }

    public function updateDisplays(Request $request)
    {
        $validated = $request->validate([
            'lcd_mode' => ['required', Rule::in(['clock', 'sensor', 'custom'])],
            'lcd_custom_text' => ['nullable', 'string', 'max:32', 'required_if:lcd_mode,custom'],
            'oled_mode' => ['required', Rule::in(['clock', 'sensor', 'custom'])],
            'oled_custom_text' => ['nullable', 'string', 'max:64', 'required_if:oled_mode,custom'],
        ]);

        $state = DeviceState::singleton();
        $state->update([
            'lcd_mode' => $validated['lcd_mode'],
            'lcd_custom_text' => $validated['lcd_mode'] === 'custom' ? Arr::get($validated, 'lcd_custom_text') : null,
            'oled_mode' => $validated['oled_mode'],
            'oled_custom_text' => $validated['oled_mode'] === 'custom' ? Arr::get($validated, 'oled_custom_text') : null,
        ]);

        $state->refresh();

        return $this->respond($request, 'Display preferences updated.', $state);
    }

    public function updateAlarm(Request $request)
    {
        $validated = $request->validate([
            'alarm_enabled' => ['sometimes', 'boolean'],
            'alarm_time' => ['nullable', 'date_format:H:i'],
        ]);

        if (($validated['alarm_enabled'] ?? false) && empty($validated['alarm_time'])) {
            return $this->respond($request, 'Alarm time is required when enabling the alarm.', null, 422);
        }

        $state = DeviceState::singleton();
        $state->update([
            'alarm_enabled' => (bool) ($validated['alarm_enabled'] ?? false),
            'alarm_time' => $validated['alarm_time'] ?? $state->alarm_time,
        ]);

        $state->refresh();

        return $this->respond($request, 'Alarm settings saved.', $state);
    }

    public function updateOutputs(Request $request)
    {
        $validated = $request->validate([
            'led_mode' => ['required', Rule::in(['off', 'on', 'auto'])],
            'buzzer_enabled' => ['sometimes', 'boolean'],
        ]);

        $state = DeviceState::singleton();
        $state->update([
            'led_mode' => $validated['led_mode'],
            'buzzer_enabled' => (bool) ($validated['buzzer_enabled'] ?? false),
        ]);

        $state->refresh();

        return $this->respond($request, 'Output controls updated.', $state);
    }

    private function respond(Request $request, string $message, ?DeviceState $state = null, int $status = 200)
    {
        if ($request->expectsJson()) {
            return response()->json([
                'message' => $message,
                'state' => $state,
            ], $status);
        }

        /** @var RedirectResponse $redirect */
        $redirect = back()->with('status', $message);

        return $redirect;
    }
}
