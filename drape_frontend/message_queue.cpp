#include "drape_frontend/message_queue.hpp"

#include "base/assert.hpp"
#include "base/stl_add.hpp"

namespace df
{

MessageQueue::~MessageQueue()
{
  CancelWait();
  ClearQuery();
}

dp::TransferPointer<Message> MessageQueue::PopMessage(unsigned maxTimeWait)
{
  threads::ConditionGuard guard(m_condition);

  WaitMessage(maxTimeWait);

  /// even waitNonEmpty == true m_messages can be empty after WaitMessage call
  /// if application preparing to close and CancelWait been called
  if (m_messages.empty())
    return dp::MovePointer<Message>(NULL);

  dp::MasterPointer<Message> msg = m_messages.front();
  m_messages.pop_front();
  return msg.Move();
}

void MessageQueue::PushMessage(dp::TransferPointer<Message> message)
{
  threads::ConditionGuard guard(m_condition);

  bool wasEmpty = m_messages.empty();
  m_messages.push_back(dp::MasterPointer<Message>(message));

  if (wasEmpty)
    guard.Signal();
}

void MessageQueue::WaitMessage(unsigned maxTimeWait)
{
  if (m_messages.empty())
    m_condition.Wait(maxTimeWait);
}

void MessageQueue::CancelWait()
{
  m_condition.Signal();
}

void MessageQueue::ClearQuery()
{
  DeleteRange(m_messages, dp::MasterPointerDeleter());
}

} // namespace df
