# Architettura

```mermaid
flowchart LR
    %% Subgraph encapsulating the target data structures

    %% Core state and logic components
    sctrt[sctrt]
    state_node[state]
    core[core]
    tb[tb]
    profiler[profiler]

    subgraph DS [ ]
        direction TB
        euid_hash[euid_hash]
        str_hash[str_hash]
        sc_bitmap[sc_bitmap]
    end

    %% System Call Table and Hooking Context
    subgraph HK [ ]
        direction TB
        hook[hook]
        hook_ctxsaver[hook_ctxsaver]
    end
    
    %% Device and IOCTL interfaces
    subgraph DEV [ ]
        direction TB
        dev_ioctl[dev_ioctl]
        dev[dev]
    end
    

    %% Edge Definitions (State)
    state_node --> DS
    state_node <--> hook
    state_node --> tb
    
    %% Edge Definitions (Device IOCTL)
    dev_ioctl --> state_node
    dev_ioctl --> profiler
    dev --> dev_ioctl
    
    %% Edge Definitions (Hooks)
    hook --> tb
    hook --> hook_ctxsaver
    hook --> core
    hook --> profiler
    
    %% Edge Definitions (Core)
    core <--> state_node
    core <--> tb
    
    %% Edge Definitions (Throttler)
    sctrt --> state_node
    sctrt --> dev
    sctrt --> hook_ctxsaver
    sctrt --> core
    sctrt --> profiler

    tb --> profiler
```

## Core
Per maggiori informazioni, vedere [Documentazione core](arch/core.md)

## Token Bucket
Per maggiori informazioni, vedere [Documentazione Token Bucket](arch/tb.md)

## Probes
Per maggiori informazioni, vedere [Documentazione Probes](arch/probes.md)

## Device
Per maggiori informazioni, vedere [Documentazione Device](arch/devices.md)

## Profiler
Per maggiori informazioni, vedere [Documentazione Profiler](arch/profiler.md)