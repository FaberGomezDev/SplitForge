# Plantilla de adaptador por juego — ESTADO: NO VALIDADO

> Copiar esta plantilla por título y **mantener el estado NO VALIDADO hasta completar las pruebas físicas**.

## Identificación

- Título:
- Steam AppID:
- Build ID (observado):
- Fecha, sistema operativo y ejecutable real:
- Identidad/licencia legal requerida (sin nombres ni IDs):
- Restricciones oficiales/EULA/antitrampas:

## Capacidades verificadas

| Prueba | Resultado | Evidencia / condiciones |
| --- | --- | --- |
| Iniciar 1 cliente oficial | Pendiente | |
| Dos clientes autorizados en una PC | Pendiente | |
| Un mismo mundo/partida | Pendiente | |
| LAN sin Internet real | Pendiente | |
| 2 mandos sin entrada cruzada | Pendiente | |
| Dos ventanas se restauran | Pendiente | |
| Guardados intactos tras cerrar/fallo | Pendiente | |
| Rendimiento medido | Pendiente | |

## Preflight y bloqueo

- Comprobaciones que nunca deben saltarse:
- Qué mostrar cuando falta licencia, cuenta o servidor:
- Riesgos conocidos y cómo aislarlos:
- Modo offline explícitamente admitido / no admitido / desconocido:

## Ciclo de sesión (propuesto)

1. `preflight`: solo lectura, fallos con motivo.
2. `prepare`: validar backups de cualquier archivo que pudiera editarse.
3. `start`: abrir clientes **autorizados**, servidor si procede.
4. `monitor`: estados de cliente, dispositivo, guardado y métricas.
5. `stop`: cierre normal, no matar juegos ajenos.
6. `restore`: revertir posiciones/configuración propia.

## Datos de validación

- Configuración y fecha:
- Procedimiento reproducible:
- Pruebas automatizadas:
- Prueba manual de dos mandos:
- FPS/frametimes/RAM/VRAM:
- Incidentes y rollback:
- Resultado final: `NO VALIDADO` / `BLOQUEADO` / `EXPERIMENTAL` / `COMPATIBLE`:
