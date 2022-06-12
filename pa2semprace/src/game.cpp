#include "game.hpp"
#include "text.hpp"


using namespace std;

CGame::CGame( int *argcPtr, char *argv[] )
        : m_window( argcPtr, argv ),
          m_painter( [ this ](){ redraw(); } ),
          m_levelLoader( m_window,
                         m_engine,
                         m_objects,
                         m_text,
                         m_painter,
                         "assets/tutorial_1.json" )
{
  m_levelLoader.loadLevel();

  m_window.registerDrawEvent( this, &CGame::redraw );
  m_window.registerKeyEvent( this, &CGame::keyPress );
  m_window.registerMouseButtonEvent( this, &CGame::clickHandler );
  m_window.registerMotionButtonEvent( this, &CGame::moveHandler );
}

CGame::~CGame()
{
  for( const auto &obj: m_objects )
    delete obj;
  for( const auto &text: m_text )
    delete text;
}

void CGame::nextFrame()
{
  if( m_paused )
  {
    redraw();
    return;
  }

  vector<TManifold> collisions = m_engine.step( m_objects, frameLength / 1000 );
  if( checkPlayerHealth() )
  {
    m_levelLoader.loadLevel( EActionType::resetLevel );
    pause();
  }
  else if( checkCollisions( collisions ) )
  {
    m_levelLoader.loadLevel( EActionType::nextLevel );
    pause();
  }
  if( m_levelLoader.healthBar && !playerOnScreen() )
    keyPress( 'r', 0, 0 );
  setTimer();
  redraw();
}

void CGame::start()
{
  m_paused = false;
  m_painter.stop();
  setTimer();
}

void CGame::pause()
{
  m_paused = true;
  redraw();
}

void CGame::redraw()
{
  // Clear Color and Depth Buffers
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Reset transformations
  glLoadIdentity();

  for( const auto &obj: m_objects )
    obj->render( m_window );

  if( !m_engine.frame )
    for( const auto &text: m_text )
      text->render( m_window );

  if( m_levelLoader.healthBar )
    drawHealthBar();

  glutSwapBuffers();
}

void CGame::drawHealthBar()
{
  for( const auto &obj: m_objects )
  {
    if( !( obj->m_tag & ETag::PLAYER ) )
      continue;

    TVector<2> screenSize = m_window.getViewSize();

    TVector<2> barBottom = { screenSize[ 0 ] / 25, screenSize[ 1 ] / 20 };
    TVector<2> fullBar = screenSize * 0.9;
    fullBar[ 0 ] = 0;

    m_window.drawLine( barBottom,
                       barBottom + fullBar * obj->m_attributes.integrity,
                       20, ETag::HEALTH );
  }
}


void CGame::mainLoop()
{
  m_window.mainLoop();
}

void CGame::keyPress( unsigned char key, int, int )
{
  if( key == 'p' )
    m_paused ? start() : pause();
  if( key == 'q' )
    glutLeaveMainLoop();
  if( key == 'r' )
  {
    m_levelLoader.loadLevel( EActionType::resetLevel );
    pause();
    pressed = false;
  }
}

void CGame::clickHandler( int button, int state, int x, int y )
{
  if( button == GLUT_LEFT_BUTTON )
  {
    if( state == GLUT_DOWN )
    {
      m_painter.addPoint( x, y, m_objects );
      pause();
      pressed = true;
    }
    else if( state == GLUT_UP && pressed )
    {
      m_painter.stop( x, y, m_objects );
      start();
      pressed = false;
    }
  }
}

void CGame::moveHandler( int x, int y )
{
  if( pressed )
    m_painter.addPoint( x, y, m_objects );
}

bool CGame::checkCollisions( const vector<TManifold> &collisions )
{
  return any_of( collisions.begin(), collisions.end(), []( const auto &collision )
  {
    return checkCollision( collision );
  } );
}

bool CGame::checkCollision( const TManifold &collision )
{
  return ( ( collision.first->m_tag & ETag::TARGET ) &&
           ( collision.second->m_tag & ETag::PLAYER ) ) ||
         ( ( collision.first->m_tag & ETag::PLAYER ) &&
           ( collision.second->m_tag & ETag::TARGET ) );
}

bool CGame::checkPlayerHealth() const
{
  return any_of( m_objects.begin(), m_objects.end(),
                 []( const auto &obj )
                 {
                   return obj->m_tag & ETag::PLAYER &&
                          obj->m_attributes.integrity <= 0;
                 } );
}

long CGame::getMilliseconds()
{
  return chrono::duration_cast<chrono::milliseconds>( chrono::system_clock::now().time_since_epoch() ).count();
}

void CGame::setTimer()
{
  auto currentTime = getMilliseconds();
  long timeDiff = currentTime - lastFrame;
  lastFrame = currentTime;

  long sleepTime = (long)frameLength - timeDiff;
  m_window.registerTimerEvent( this, &CGame::nextFrame, max( sleepTime, 0l ) );
}

bool CGame::playerOnScreen()
{
  for( const auto &obj: m_objects )
  {
    if( !( obj->m_tag & ETag::PLAYER ) )
      continue;

    TVector<2> screenSize = m_window.getViewSize();
    if( obj->m_position[ 0 ] < 0 || obj->m_position[ 1 ] < 0
        || obj->m_position[ 0 ] > screenSize[ 0 ] || obj->m_position[ 1 ] > screenSize[ 1 ] )
      return false;
  }
  return true;
}
