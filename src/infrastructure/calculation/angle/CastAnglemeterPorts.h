#ifndef CLEANGRADUATOR_CASTANGLEMETERPORTS_H
#define CLEANGRADUATOR_CASTANGLEMETERPORTS_H

namespace domain::ports {
   struct ILogger;
}

namespace infra::calculation {
   struct CastAnglemeterPorts {
      domain::ports::ILogger &logger;
   };
}

#endif //CLEANGRADUATOR_CASTANGLEMETERPORTS_H