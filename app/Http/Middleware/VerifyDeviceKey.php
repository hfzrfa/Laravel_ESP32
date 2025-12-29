<?php

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;

class VerifyDeviceKey
{
    public function handle(Request $request, Closure $next): Response
    {
        $expected = config('services.device.api_key');
        $provided = $request->header('X-Device-Key') ?? $request->query('device-key');

        if (empty($expected)) {
            abort(503, 'Device API key is not configured.');
        }

        if (!$provided || !hash_equals($expected, (string) $provided)) {
            abort(401, 'Invalid device API key.');
        }

        return $next($request);
    }
}
