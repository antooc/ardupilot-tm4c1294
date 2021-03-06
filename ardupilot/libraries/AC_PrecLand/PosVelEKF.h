#ifndef __AC_PrecLand_PosVelEKF_h__
#define __AC_PrecLand_PosVelEKF_h__


class PosVelEKF {
public:
    void init(float pos, float posVar, float vel, float velVar);
    void predict(float dt, float dVel, float dVelNoise);
    void fusePos(float pos, float posVar);
    void fuseVel(float vel, float velVar);

    float getPos() const { return _state[0]; }
    float getVel() const { return _state[1]; }

    float getPosNIS(float pos, float posVar);

private:
    float _state[2];
    float _cov[3];
};

#endif /* __AC_PrecLand_PosVelEKF_h__ */