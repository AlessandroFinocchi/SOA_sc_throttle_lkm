# Architettura

```tikz
\usetikzlibrary{positioning, arrows.meta, calc, fit, backgrounds}
\begin{document}
\begin{tikzpicture}[
    >=Stealth,
    % Definizione degli stili per i blocchi
    box/.style={draw, fill=gray!20, minimum height=0.8cm, minimum width=1.5cm, font=\ttfamily, thick, align=center},
    outerbox/.style={draw, fill=gray!10, inner sep=12pt, thick}
]

% 1. Strutture Dati (Blocco Hash a sinistra)
\node[box] (str_hash) at (-3.5, 2.5) {str\_hash};
\node[box] (euid_hash) [above=0.2cm of str_hash] {euid\_hash};
\node[box] (sc_bitmap) [below=0.2cm of str_hash] {sc\_bitmap};

% Bounding box per il gruppo hash
\begin{scope}[on background layer]
    \node[outerbox, fit=(euid_hash) (str_hash) (sc_bitmap)] (hash_block) {};
\end{scope}

% 2. Nodi di Controllo Principali
\node[box] (state) at (0, 4.5) {state};
\node[box] (core) at (0, 0) {core};

% 3. Nodi Intermedi e Sottosistema Hook
\node[box] (tb) at (4.5, 2.5) {tb};
\node[box] (hook) at (4.5, 0) {hook};

% 4. Nodi Dispositivo (IOCTL)
\node[box] (dev_ioctl) at (4.5, 4) {dev\_ioctl};
\node[box] (dev) at (7.5, 4) {dev};

% 5. Context Saver e Dispatcher Principale (sctrt)
\node[box] (hook_ctxsaver) at (7.5, 0) {hook\_ctxsaver};
\node[box, minimum height=2.2cm] (sctrt) at (11, 2.5) {sctrt};


% ==========================================
% ARCHI (Routing del Controllo e dei Dati)
% ==========================================

% Interazioni State <-> Blocco Hash
\draw[->, thick] (hash_block.north east) -- (state.west);
\draw[->, thick] (state.south west) -- (hash_block.east);

% State -> Core (Diretto)
\draw[->, thick] (state.south) -- (core.north);

% Flussi curvi asincroni verso tb e hook
\draw[->, thick] ([xshift=0.2cm]state.south) to[out=-70, in=180] (tb.west);
\draw[->, thick] ([xshift=-0.2cm]state.south) to[out=-90, in=160] (hook.west);
\draw[->, thick] (core.north east) to[out=45, in=180] (tb.west);

% Flussi interni al sottosistema hook
\draw[->, thick] (hook.north) -- (tb.south);
\draw[->, thick] (hook.west) -- (core.east);
\draw[->, thick] (hook.east) -- (hook_ctxsaver.west);

% Routing Dispositivo / IOCTL
\draw[->, thick] (dev.west) -- (dev_ioctl.east);
% dev_ioctl va a sinistra, poi scende leggermente per entrare nel lato destro di state
\draw[->, thick] (dev_ioctl.west) -- ++(-1.0, 0) |- ([yshift=-0.2cm]state.east);

% Routing del Dispatcher (sctrt)
% Verso dev (Ortogonale, esce dall'alto a sinistra)
\draw[->, thick] (sctrt.north west) |- (dev.east);
% Verso state (Lungo: esce dall'alto, supera i nodi, scende in state)
\draw[->, thick] (sctrt.north) -- ++(0, 1.5) -| (state.north);
% Verso hook_ctxsaver (Ortogonale, esce dal basso a sinistra)
\draw[->, thick] (sctrt.south west) |- (hook_ctxsaver.east);
% Verso core (Lungo: esce dal basso, passa sotto, sale in core)
\draw[->, thick] (sctrt.south) -- ++(0, -1.5) -| (core.south);

\end{tikzpicture}
\end{document}
```

## Devices
Per maggiori informazioni, vedere [Documentazione Devices](arch/devices.md)


## Probes

## Services

## Utils

## Tests