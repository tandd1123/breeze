﻿#include "sceneModule.h"
#include "../avatarService.h"
#include <ProtoSceneServer.h>

void SceneModule::onModuleInit(AvatarService & avatar)
{
    avatar.slotting<SceneGroupInfoNotice>(std::bind(&SceneModule::onSceneGroupInfoNotice, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupGetReq>(std::bind(&SceneModule::onSceneGroupGetReq, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupEnterReq>(std::bind(&SceneModule::onSceneGroupEnterReq, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupCancelReq>(std::bind(&SceneModule::onSceneGroupCancelReq, this, std::ref(avatar), _1, _2));

    avatar.slotting<SceneGroupCreateReq>(std::bind(&SceneModule::onSceneGroupCreateReq, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupJoinReq>(std::bind(&SceneModule::onSceneGroupJoinReq, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupInviteReq>(std::bind(&SceneModule::onSceneGroupInviteReq, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupRejectReq>(std::bind(&SceneModule::onSceneGroupRejectReq, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupLeaveReq>(std::bind(&SceneModule::onSceneGroupLeaveReq, this, std::ref(avatar), _1, _2));

    avatar.slotting<SceneGroupGetResp>(std::bind(&SceneModule::onSceneGroupGetStatusResp, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupEnterResp>(std::bind(&SceneModule::onSceneGroupEnterResp, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupCancelResp>(std::bind(&SceneModule::onSceneGroupCancelResp, this, std::ref(avatar), _1, _2));

    avatar.slotting<SceneServerJoinGroupAck>(std::bind(&SceneModule::onSceneServerJoinGroupAck, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupInviteResp>(std::bind(&SceneModule::onSceneGroupInviteResp, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupInviteNotice>(std::bind(&SceneModule::onSceneGroupInviteNotice, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupRejectResp>(std::bind(&SceneModule::onSceneGroupRejectResp, this, std::ref(avatar), _1, _2));
    avatar.slotting<SceneGroupLeaveResp>(std::bind(&SceneModule::onSceneGroupLeaveResp, this, std::ref(avatar), _1, _2));
}

void SceneModule::onSceneGroupInfoNotice(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream &rs)
{
    avatar.toService(STClient, trace.oob, rs.getStream(), rs.getStreamLen());
}

void SceneModule::onSceneGroupGetReq(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream &rs)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        LOGW("STWorldMgr service not open. " << trace);
        avatar.toService(STClient, trace.oob, SceneGroupGetResp(EC_SERVICE_NOT_OPEN));
        return;
    }
    avatar.toService(STWorldMgr, trace.oob, rs.getStream(), rs.getStreamLen());
}

void SceneModule::onSceneGroupEnterReq(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        LOGW("STWorldMgr service not open. " << trace);
        avatar.toService(STClient, trace.oob, SceneGroupEnterResp(EC_SERVICE_NOT_OPEN));
        return;
    }
    avatar.toService(STWorldMgr, trace.oob, rs.getStream(), rs.getStreamLen());
}
void SceneModule::onSceneGroupCancelReq(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        LOGW("STWorldMgr service not open. " << trace);
        avatar.toService(STClient, trace.oob, SceneGroupCancelResp(EC_SERVICE_NOT_OPEN));
        return;
    }
    avatar.toService(STWorldMgr, trace.oob, rs.getStream(), rs.getStreamLen());
}
void SceneModule::fillGroupInfo(AvatarService & avatar, SceneServerJoinGroupIns& ins)
{
    ins.groupID = InvalidGroupID;
    ins.refresh = 1;
    ins.baseInfo.avatarID = avatar._baseInfo._data.avatarID;
    ins.baseInfo.avatarName = avatar._baseInfo._data.avatarName;
    ins.baseInfo.eid = InvalidEntityID;
    ins.baseInfo.exp = 0;
    ins.baseInfo.gold = 0;
    ins.baseInfo.iconID = avatar._baseInfo._data.iconID;
    ins.baseInfo.level = 1;
    ins.baseInfo.modelID = avatar._baseInfo._data.modeID;
    ins.baseInfo.modelName = "";
    ins.baseInfo.state = ENTITY_STATE_ACTIVE;
    ins.baseInfo.camp = ENTITY_CAMP_NONE;
    ins.baseInfo.etype = ENTITY_PLAYER;
    if (true)
    {
        auto result = DBDict::getRef().getOneKeyEntityBase(avatar._baseInfo._data.modeID);
        if (!result.first)
        {
            ins.baseInfo.exp = result.second.exp;
            ins.baseInfo.gold = result.second.gold;
            ins.baseInfo.level = result.second.level;
            ins.baseInfo.modelName = result.second.modelName;
        }

    }

    if (true)
    {
        auto result = DBDict::getRef().getTwoKeyEntityProp(avatar._baseInfo._data.modeID, 0);
        if (!result.first)
        {
            ins.fixedProps = result.second;
        }
        result = DBDict::getRef().getTwoKeyEntityProp(avatar._baseInfo._data.modeID, 1);
        if (!result.first)
        {
            ins.growthProps = result.second;
        }
        result = DBDict::getRef().getTwoKeyEntityProp(avatar._baseInfo._data.modeID, 2);
        if (!result.first)
        {
            ins.growths = result.second;
        }
        if (ins.fixedProps.hp <= 0)
        {
            ins.fixedProps.hp = 1000;
        }
        if (ins.fixedProps.attack <= 0)
        {
            ins.fixedProps.attack = 10;
        }
    }
}
void SceneModule::refreshGroupInfo(AvatarService & avatar)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        return;
    }
    SceneServerJoinGroupIns ins;
    fillGroupInfo(avatar, ins);
    OutOfBand oob;
    oob.clientAvatarID = avatar.getServiceID();
    avatar.toService(STWorldMgr, oob, ins);
}
void SceneModule::onSceneGroupCreateReq(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        LOGW("STWorldMgr service not open. " << trace);
        avatar.toService(STClient, trace.oob, SceneGroupCreateResp(EC_SERVICE_NOT_OPEN));
        return;
    }
    SceneServerJoinGroupIns ins;
    fillGroupInfo(avatar, ins);
    ins.groupID = InvalidGroupID;
    ins.refresh = 0;
    avatar.toService(STWorldMgr, trace.oob, ins);
}

void SceneModule::onSceneGroupJoinReq(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        LOGW("STWorldMgr service not open. " << trace);
        avatar.toService(STClient, trace.oob, SceneGroupJoinResp(EC_SERVICE_NOT_OPEN));
        return;
    }
    SceneGroupJoinReq req;
    rs >> req;
    SceneServerJoinGroupIns ins;
    fillGroupInfo(avatar, ins);
    ins.groupID = req.groupID;
    ins.refresh = 0;
    avatar.toService(STWorldMgr, trace.oob, ins);
}
void SceneModule::onSceneGroupInviteReq(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        LOGW("STWorldMgr service not open. " << trace);
        avatar.toService(STClient, trace.oob, SceneGroupInviteResp(EC_SERVICE_NOT_OPEN));
        return;
    }
    avatar.toService(STWorldMgr, trace.oob, rs.getStream(), rs.getStreamLen());
}
void SceneModule::onSceneGroupRejectReq(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        LOGW("STWorldMgr service not open. " << trace);
        avatar.toService(STClient, trace.oob, SceneGroupRejectResp(EC_SERVICE_NOT_OPEN));
        return;
    }
    avatar.toService(STWorldMgr, trace.oob, rs.getStream(), rs.getStreamLen());
}
void SceneModule::onSceneGroupLeaveReq(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    if (!Docker::getRef().peekService(STWorldMgr, InvalidServiceID))
    {
        LOGW("STWorldMgr service not open. " << trace);
        avatar.toService(STClient, trace.oob, SceneGroupGetResp(EC_SERVICE_NOT_OPEN));
        return;
    }
    avatar.toService(STWorldMgr, trace.oob, rs.getStream(), rs.getStreamLen());
}

void SceneModule::onSceneGroupGetStatusResp(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    avatar.toService(STClient, trace.oob, rs.getStream(), rs.getStreamLen());
}

void SceneModule::onSceneGroupEnterResp(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    avatar.toService(STClient, trace.oob, rs.getStream(), rs.getStreamLen());
}

void SceneModule::onSceneGroupCancelResp(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    avatar.toService(STClient, trace.oob, rs.getStream(), rs.getStreamLen());
}


void SceneModule::onSceneServerJoinGroupAck(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    SceneServerJoinGroupAck ack;
    rs >> ack;
    if (ack.oldGroupID == InvalidGroupID)
    {
        avatar.toService(STClient, trace.oob, SceneGroupCreateResp(ack.retCode));
    }
    else
    {
        avatar.toService(STClient, trace.oob, SceneGroupJoinResp(ack.retCode));
    }
}

void SceneModule::onSceneGroupInviteResp(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    avatar.toService(STClient, trace.oob, rs.getStream(), rs.getStreamLen());
}

void SceneModule::onSceneGroupInviteNotice(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    avatar.toService(STClient, trace.oob, rs.getStream(), rs.getStreamLen());
}

void SceneModule::onSceneGroupRejectResp(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    avatar.toService(STClient, trace.oob, rs.getStream(), rs.getStreamLen());
}

void SceneModule::onSceneGroupLeaveResp(AvatarService & avatar, const Tracing & trace, zsummer::proto4z::ReadStream & rs)
{
    avatar.toService(STClient, trace.oob, rs.getStream(), rs.getStreamLen());
}










