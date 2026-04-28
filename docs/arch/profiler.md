# Profiler DA RIFARE DA CAPOOOOOOO

## Scelta di sincronizzazione
Per i dati sul numero massimo e medio di thread si è scelto di usare variabili atomiche perchè ...

Per i dati sul massimo picco si è scelto si usare Seqlock_t perchè questi dati vengono scritti raramente: si può infatti pensare che dopo una prima fase di scritture intense, si raggiunga un picco che difficilmente venga superato, motivo per cui si è abbandonata l'idea di usare variabili per-CPU nell'ottica che questi dati possano essere read-intensive, e quindi per non spostare l'onere sui lettori.