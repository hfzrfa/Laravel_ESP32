<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\DeviceState;
use App\Models\SensorReading;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;

class SensorReadingController extends Controller
{
    public function store(Request $request): JsonResponse
    {
        $validated = $request->validate([
            'temperature_c' => ['required', 'numeric', 'between:-40,125'],
            'humidity' => ['required', 'numeric', 'between:0,100'],
            'battery_level' => ['nullable', 'numeric', 'between:0,100'],
            'signal_strength' => ['nullable', 'integer', 'between:-120,0'],
            'recorded_at' => ['nullable', 'date'],
        ]);

        $reading = SensorReading::create($validated);

        $state = DeviceState::singleton();
        $state->update([
            'last_ping_at' => now(),
            'last_reported_ip' => $request->ip(),
        ]);

        return response()->json([
            'message' => 'Reading stored.',
            'reading_id' => $reading->id,
            'next_state' => $state->fresh(),
        ]);
    }

    public function latest(): JsonResponse
    {
        $reading = SensorReading::orderByDesc('recorded_at')
            ->orderByDesc('created_at')
            ->first();

        return response()->json([
            'data' => $reading,
        ]);
    }
}
