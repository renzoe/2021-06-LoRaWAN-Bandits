/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Renzo E. Navas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Renzo E. Navas <renzo.navas@inria.fr>
 */

#include "bandit-delayed-reward-intelligence.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("BanditDelayedRewardIntelligence");
NS_OBJECT_ENSURE_REGISTERED (BanditDelayedRewardIntelligence);

TypeId BanditDelayedRewardIntelligence::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BanditDelayedRewardIntelligence")
    .SetGroupName ("lorawan")
    .AddConstructor<BanditDelayedRewardIntelligence> ()
    ;
  return tid;
}
BanditDelayedRewardIntelligence::BanditDelayedRewardIntelligence ()
{
  // TODO Auto-generated constructor stub

}

BanditDelayedRewardIntelligence::~BanditDelayedRewardIntelligence ()
{
  // TODO Auto-generated destructor stub
}


void
BanditDelayedRewardIntelligence::InitBanditAgentAndArms (
    Ptr<AdrBanditAgent> adrBanditAgent)
{
  this->m_adrBanditAgent = adrBanditAgent;
  this->CleanArmsStats();
  this->m_banditNeedsStats = true;
  this->m_waitingForStats = false;

  //TODO: initialize arms use reward vector
}



void
BanditDelayedRewardIntelligence::UpdateUsedArm (size_t armNumber,  int frameCnt)
{
  ++m_armsAndRewards[armNumber][0];

  if (frameCnt > m_frmCntMaxWithoutStats) m_frmCntMaxWithoutStats = frameCnt;

  // here we could also put the logic to set the m_needStats boolean
}

Ptr<BanditRewardReq>
BanditDelayedRewardIntelligence::GetRewardsMacCommandReq (uint16_t currentFrame)
{

  m_requestedMaxFrmCntReward = currentFrame; // Important to keep track, because in case of re-sent message m_requestedMaxFrmCntReward will be < m_frmCntMaxWithoutStats

  uint8_t frameDelta =  currentFrame - m_frmCntMinWithoutStats;
  // Colored Terminal: https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
  NS_LOG_INFO("\033[1;31m");
  NS_LOG_INFO("GetRewardsReq -- frameDelta = " << unsigned(frameDelta) << "  currentFrame: " << currentFrame);

  Ptr<BanditRewardReq> req = CreateObject<BanditRewardReq> (currentFrame, frameDelta);
  m_waitingForStats= true;

  NS_LOG_INFO("\033[0m");
  return req;
}

void
BanditDelayedRewardIntelligence::UpdateRewardsAns (
    Ptr<BanditRewardAns> delayedRewardsAns)
{


  //m_frmCntMaxWithoutStats

  //if (m_requestedMaxFrmCntReward == currentFCnt) ...

  std::vector<int> drStatistics = delayedRewardsAns->GetDataRateStatistics();

  for (size_t i = 0; i < m_adrBanditAgent->GetNumberOfArms() ; i++)
    {
      m_armsAndRewards[i][0];// if not zero:

      m_armsAndRewards[i][1];//= //drStatistics[i] / m_armsAndRewards[i][0]
    }

  //this->m_adrBanditAgent->UpdateReward(m_dataRate, cost_for_arm[m_dataRate]);

  m_frmCntMinWithoutStats = m_requestedMaxFrmCntReward+1;

  //m_frmCntMinWithoutStats = currentFCnt+1;

}


void
BanditDelayedRewardIntelligence::CleanArmsStats ()
{
  // https://stackoverflow.com/questions/3948290/what-is-the-safe-way-to-fill-multidimensional-array-using-stdfill

  std::fill( &m_armsAndRewards[0][0], &m_armsAndRewards[0][0] + sizeof(m_armsAndRewards) /* / sizeof(flags[0][0]) */, 0 );
}


bool
BanditDelayedRewardIntelligence::isGetRewardsMacCommandReqNeeded () const
{
  return m_banditNeedsStats;
}


bool
BanditDelayedRewardIntelligence::isBanditNeedsStats () const
{
  return m_banditNeedsStats;
}

void
BanditDelayedRewardIntelligence::setBanditNeedsStats (
    bool mBanditNeedsStats)
{
  m_banditNeedsStats = mBanditNeedsStats;
}

}
}
