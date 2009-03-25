# Roadmap

## Accepted features
  * event based -- (observe-notify) notification center in some way
  * libev for I/O

## Under consideration
  * multi-protocol -- HTTP being the premiere one


# Indended setups

## A. Proxy-based balancing

Number of hosts are abstract.

One or more workers horizontally spread out over one or more hosts.

           internet 
              |
            proxy
        ___/  |  \___
       /      |      \
    worker  worker  worker


## B. DNS-based balancing

Number of hosts are concrete.

One or more workers on every host spread out horizontally.

           internet
              |
           DNS/SRV
        ___/  |  \___
       /      |      \
    worker  worker  worker


## C. Stand-alone

Simplest setup. Primarily for development and internal deployment.

           internet
              |
              |
           worker

