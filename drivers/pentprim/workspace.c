#include "drv.h"

BR_ALIGNAS(8) struct _workspace workspace               = {0};
BR_ALIGNAS(8) struct ArbitraryWidthWorkspace workspaceA = {0};

BR_STATIC_ASSERT((offsetof(struct _workspace, colour) % 8) == 0, "colour must be 8-byte aligned");
BR_STATIC_ASSERT((offsetof(struct _workspace, scratch0) % 8) == 0, "scratch0 must be 8-byte aligned");

BR_STATIC_ASSERT(sizeof(struct _workspace) == 320, "struct _workspace must be 320 bytes");
BR_STATIC_ASSERT(sizeof(struct ArbitraryWidthWorkspace) == 116, "struct ArbitraryWidthWorkspace must be 116 bytes");
