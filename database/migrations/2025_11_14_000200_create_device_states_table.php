<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('device_states', function (Blueprint $table) {
            $table->id();
            $table->string('lcd_mode')->default('clock');
            $table->string('lcd_custom_text')->nullable();
            $table->string('oled_mode')->default('clock');
            $table->string('oled_custom_text')->nullable();
            $table->string('led_mode')->default('auto');
            $table->boolean('buzzer_enabled')->default(false);
            $table->time('alarm_time')->nullable();
            $table->boolean('alarm_enabled')->default(false);
            $table->timestamp('alarm_last_triggered_at')->nullable();
            $table->timestamp('last_ping_at')->nullable();
            $table->string('last_reported_ip')->nullable();
            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('device_states');
    }
};
