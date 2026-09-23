ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Por favor configura la variable de entorno PVSNESLIB_HOME apuntando al directorio raíz de PVSnesLib")
endif

# Nombre y título de la ROM de salida
export ROMNAME := ProyectoSNES
export ROMTITLE := TAROUH DEMO

# --- Audio (se activa en la tarea 3 del plan, cuando haya módulos .it) ---------
# AUDIOFILES y SOUNDBANK deben declararse ANTES del include de snes_rules.
# Los .it de efectos van primero en la lista.
# export AUDIOFILES := res/musica/jingle_publisher.it res/musica/titulo.it
# export SOUNDBANK  := res/musica/soundbank

# Incluir reglas estándar de compilación de PVSnesLib
include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all clean

# Regla por defecto para compilar
all: buildWithSummary
buildActual: $(OBJS) $(ROMNAME).sfc

# Regla para limpiar los binarios y recursos
clean: cleanBuildRes cleanRom cleanGfx
