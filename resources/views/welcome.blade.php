<!DOCTYPE html>
<html lang="{{ str_replace('_', '-', app()->getLocale()) }}">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>{{ config('app.name', 'ESP32 Console') }}</title>

    <link rel="preconnect" href="https://fonts.bunny.net">
    <link href="https://fonts.bunny.net/css?family=instrument-sans:400,500,600" rel="stylesheet" />

    @if (file_exists(public_path('build/manifest.json')) || file_exists(public_path('hot')))
        @vite(['resources/css/app.css', 'resources/js/app.js'])
    @endif
</head>

<body
    class="min-h-screen bg-gradient-to-br from-zinc-50 via-white to-zinc-100 text-zinc-900 antialiased dark:from-zinc-950 dark:via-zinc-900 dark:to-zinc-950">
    @php
        $connected = optional($state->last_ping_at)?->gt(now()->subMinutes(2));
        $statusBadge = $connected ? 'badge badge-success' : 'badge badge-warn';
        $statusLabel = $connected ? 'Device Online' : 'Waiting for Device';
        $lastPing = optional($state->last_ping_at)?->diffForHumans() ?? 'No heartbeat yet';
        $temperature = $latestReading->temperature_c ?? null;
        $humidity = $latestReading->humidity ?? null;
        $battery = $latestReading->battery_level ?? null;
        $signal = $latestReading->signal_strength ?? null;
        $readingTime =
            optional($latestReading->recorded_at ?? $latestReading?->created_at)?->diffForHumans() ??
            'Awaiting first packet';
    @endphp

    <div class="mx-auto flex max-w-6xl flex-col gap-6 px-4 py-10 lg:px-8">
        <header class="flex flex-col gap-4 lg:flex-row lg:items-center lg:justify-between">
            <div>
                <p class="text-xs uppercase tracking-[0.3em] text-zinc-500 dark:text-zinc-400">ESP32 real-time monitor
                </p>
                <h1 class="mt-2 text-3xl font-semibold text-zinc-900 dark:text-white">Home Comfort Console</h1>
                <p class="mt-1 text-sm text-zinc-500 dark:text-zinc-400">Track DHT11 telemetry, manage the OLED/LCD
                    content, and schedule RTC alarms directly from your network.</p>
            </div>
            <div class="flex flex-wrap items-center gap-3">
                <span class="{{ $statusBadge }}">
                    <span class="relative flex h-2.5 w-2.5">
                        <span
                            class="absolute inline-flex h-full w-full animate-ping rounded-full bg-current opacity-40"></span>
                        <span class="relative inline-flex h-2.5 w-2.5 rounded-full bg-current"></span>
                    </span>
                    {{ $statusLabel }}
                </span>
                <span
                    class="badge border-zinc-200/80 bg-white/80 text-zinc-600 dark:border-zinc-700 dark:bg-zinc-900/60 dark:text-zinc-300">
                    Last ping: {{ $lastPing }}
                </span>
                @if ($state->last_reported_ip)
                    <span
                        class="badge border-zinc-200/80 bg-white/80 text-zinc-600 dark:border-zinc-700 dark:bg-zinc-900/60 dark:text-zinc-300">
                        IP {{ $state->last_reported_ip }}
                    </span>
                @endif
            </div>
        </header>

        @if (session('status'))
            <div
                class="panel border-emerald-200/60 bg-emerald-50/70 text-sm text-emerald-900 dark:border-emerald-900/60 dark:bg-emerald-900/40 dark:text-emerald-100">
                {{ session('status') }}
            </div>
        @endif

        <section class="grid gap-6 lg:grid-cols-3">
            <div class="panel lg:col-span-2">
                <div class="flex items-center justify-between">
                    <div>
                        <p class="panel-title">Live Telemetry</p>
                        <p class="text-xs text-zinc-400">Updated {{ $readingTime }}</p>
                    </div>
                    <div class="text-xs text-zinc-400">
                        Device key protected API
                    </div>
                </div>

                <div class="mt-6 grid gap-4 md:grid-cols-2">
                    <div class="rounded-2xl bg-gradient-to-br from-indigo-500 to-purple-500 p-5 text-white shadow-xl">
                        <p class="text-sm uppercase tracking-wide text-white/80">Temperature</p>
                        <p class="stat-value mt-3 text-white">
                            {{ $temperature !== null ? number_format($temperature, 1) . '°C' : '--' }}
                        </p>
                        <p class="text-sm text-white/70">Comfort range 21-26°C</p>
                    </div>
                    <div class="rounded-2xl bg-gradient-to-br from-sky-400 to-blue-600 p-5 text-white shadow-xl">
                        <p class="text-sm uppercase tracking-wide text-white/80">Humidity</p>
                        <p class="stat-value mt-3 text-white">
                            {{ $humidity !== null ? number_format($humidity, 1) . '%' : '--' }}
                        </p>
                        <p class="text-sm text-white/70">Optimal 40-60%</p>
                    </div>
                    <div
                        class="rounded-2xl border border-zinc-100/70 bg-white/80 p-5 shadow-inner shadow-black/5 dark:border-zinc-800/50 dark:bg-zinc-900/60">
                        <p class="text-sm uppercase tracking-wide text-zinc-500 dark:text-zinc-400">Battery</p>
                        <p class="stat-value mt-3 text-zinc-900 dark:text-white">
                            {{ $battery !== null ? number_format($battery, 0) . '%' : '--' }}
                        </p>
                        <p class="text-sm text-zinc-500 dark:text-zinc-400">Keep above 40% for stable Wi-Fi</p>
                    </div>
                    <div
                        class="rounded-2xl border border-zinc-100/70 bg-white/80 p-5 shadow-inner shadow-black/5 dark:border-zinc-800/50 dark:bg-zinc-900/60">
                        <p class="text-sm uppercase tracking-wide text-zinc-500 dark:text-zinc-400">Signal</p>
                        <p class="stat-value mt-3 text-zinc-900 dark:text-white">
                            {{ $signal !== null ? $signal . ' dBm' : '--' }}
                        </p>
                        <p class="text-sm text-zinc-500 dark:text-zinc-400">Closer router improves stability</p>
                    </div>
                </div>
            </div>

            <div class="panel flex flex-col gap-4">
                <div>
                    <p class="panel-title">Alarm Timeline</p>
                    <p class="text-sm text-zinc-500 dark:text-zinc-400">RTC based events</p>
                </div>
                <dl class="space-y-3 text-sm text-zinc-600 dark:text-zinc-300">
                    <div class="flex justify-between">
                        <dt>Alarm state</dt>
                        <dd class="font-semibold">{{ $state->alarm_enabled ? 'Armed' : 'Disabled' }}</dd>
                    </div>
                    <div class="flex justify-between">
                        <dt>Scheduled time</dt>
                        <dd class="font-semibold">{{ $state->alarm_time ?? '—' }}</dd>
                    </div>
                    <div class="flex justify-between">
                        <dt>Last triggered</dt>
                        <dd class="font-semibold">
                            {{ optional($state->alarm_last_triggered_at)?->diffForHumans() ?? 'Never' }}</dd>
                    </div>
                </dl>
                <div
                    class="rounded-2xl border border-dashed border-zinc-200 p-4 text-xs text-zinc-500 dark:border-zinc-700 dark:text-zinc-400">
                    Alarm acknowledgements appear once the ESP32 calls the /api/device/alarm/ack endpoint.
                </div>
            </div>
        </section>

        <section class="grid gap-6 lg:grid-cols-3">
            <form method="POST" action="{{ route('controls.displays') }}" class="panel space-y-4">
                @csrf
                <div>
                    <p class="panel-title">Display presets</p>
                    <p class="text-sm text-zinc-500 dark:text-zinc-400">Push copy to the LCD/OLED.</p>
                </div>
                <div class="space-y-2">
                    <label class="text-sm font-medium">LCD mode</label>
                    <select name="lcd_mode" class="input-control">
                        @foreach (['clock' => 'Clock', 'sensor' => 'Sensor snapshot', 'custom' => 'Custom text'] as $value => $label)
                            <option value="{{ $value }}" @selected(old('lcd_mode', $state->lcd_mode) === $value)>{{ $label }}
                            </option>
                        @endforeach
                    </select>
                    <input type="text" name="lcd_custom_text"
                        value="{{ old('lcd_custom_text', $state->lcd_custom_text) }}"
                        placeholder="16 char line when custom" class="input-control" />
                </div>
                <div class="space-y-2">
                    <label class="text-sm font-medium">OLED mode</label>
                    <select name="oled_mode" class="input-control">
                        @foreach (['clock' => 'Clock', 'sensor' => 'Sensor snapshot', 'custom' => 'Custom text'] as $value => $label)
                            <option value="{{ $value }}" @selected(old('oled_mode', $state->oled_mode) === $value)>{{ $label }}
                            </option>
                        @endforeach
                    </select>
                    <textarea name="oled_custom_text" rows="2" class="input-control" placeholder="64 char marquee">{{ old('oled_custom_text', $state->oled_custom_text) }}</textarea>
                </div>
                <div class="flex justify-end gap-3">
                    <button type="reset" class="btn-ghost">Reset</button>
                    <button type="submit" class="btn-primary">Update displays</button>
                </div>
            </form>

            <form method="POST" action="{{ route('controls.alarm') }}" class="panel space-y-4">
                @csrf
                <div>
                    <p class="panel-title">RTC alarm</p>
                    <p class="text-sm text-zinc-500 dark:text-zinc-400">Schedule wake-up pulses.</p>
                </div>
                <div class="space-y-2">
                    <label class="inline-flex items-center gap-2 text-sm font-medium">
                        <input type="checkbox" name="alarm_enabled" value="1"
                            class="h-4 w-4 rounded border-zinc-300 text-indigo-500 focus:ring-indigo-400"
                            @checked(old('alarm_enabled', $state->alarm_enabled)) />
                        Enable alarm
                    </label>
                </div>
                <div class="space-y-2">
                    <label class="text-sm font-medium">Alarm time (24h)</label>
                    <input type="time" name="alarm_time" value="{{ old('alarm_time', $state->alarm_time) }}"
                        class="input-control" />
                </div>
                <div
                    class="rounded-2xl border border-dashed border-zinc-200 p-4 text-xs text-zinc-500 dark:border-zinc-700 dark:text-zinc-400">
                    When the ESP32 reaches the alarm time it will buzz/flash then call the acknowledgement endpoint.
                </div>
                <div class="flex justify-end">
                    <button type="submit" class="btn-primary">Save alarm</button>
                </div>
            </form>

            <form method="POST" action="{{ route('controls.outputs') }}" class="panel space-y-4">
                @csrf
                <div>
                    <p class="panel-title">Outputs</p>
                    <p class="text-sm text-zinc-500 dark:text-zinc-400">LED + buzzer policy.</p>
                </div>
                <div class="space-y-2">
                    <label class="text-sm font-medium">LED Mode</label>
                    <select name="led_mode" class="input-control">
                        @foreach (['auto' => 'Auto (sensors)', 'on' => 'Force on', 'off' => 'Force off'] as $value => $label)
                            <option value="{{ $value }}" @selected(old('led_mode', $state->led_mode) === $value)>{{ $label }}
                            </option>
                        @endforeach
                    </select>
                </div>
                <div class="space-y-2">
                    <label class="inline-flex items-center gap-2 text-sm font-medium">
                        <input type="checkbox" name="buzzer_enabled" value="1"
                            class="h-4 w-4 rounded border-zinc-300 text-indigo-500 focus:ring-indigo-400"
                            @checked(old('buzzer_enabled', $state->buzzer_enabled)) />
                        Allow buzzer alerts
                    </label>
                </div>
                <div
                    class="rounded-2xl border border-dashed border-zinc-200 p-4 text-xs text-zinc-500 dark:border-zinc-700 dark:text-zinc-400">
                    Auto mode pulses the LED when humidity &gt; 65% or temperature &gt; 30°C. Manual overrides apply
                    instantly after saving.
                </div>
                <div class="flex justify-end">
                    <button type="submit" class="btn-primary">Sync outputs</button>
                </div>
            </form>
        </section>

        <footer class="pb-10 text-center text-xs text-zinc-500 dark:text-zinc-400">
            Secured with the <code>DEVICE_API_KEY</code>. Keep the ESP32 on the same Wi-Fi network for instant updates.
        </footer>
    </div>
</body>

</html>
